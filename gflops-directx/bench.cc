
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#include <memory>
#include <vector>

#include "FMLA.h"

#define HR_CHECK(hr, name)                  \
    if (FAILED(hr)) {                       \
        printf("%s Error: %x\n", name, hr); \
        return hr;                          \
    }

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=nullptr; } }
#define UP_DIV(x, y) (((int)(x) + (int)(y) - (1)) / (int)(y))
#define ROUND_UP(x, y) (((int)(x) + (int)(y) - (1)) / (int)(y) * (int)(y))

std::shared_ptr<ID3D11Device> g_device         = nullptr;
std::shared_ptr<ID3D11DeviceContext> g_context = nullptr;
std::shared_ptr<ID3D11ComputeShader> g_fmla_cs = nullptr;

std::shared_ptr<float> in  = nullptr;
std::shared_ptr<float> out = nullptr;

std::shared_ptr<ID3D11Buffer> in_buffer    = nullptr;
std::shared_ptr<ID3D11Buffer> out_buffer   = nullptr;
std::shared_ptr<ID3D11Buffer> const_buffer = nullptr;

std::shared_ptr<ID3D11ShaderResourceView> in_srv   = nullptr;
std::shared_ptr<ID3D11UnorderedAccessView> out_uav = nullptr;

class DXTimer {
public:
    DXTimer();

    ~DXTimer();

    HRESULT Init(int count = 1);

    void Begin();

    void End();

    float TimeEclapsed();

private:
    ID3D11Query *disjoint;
    ID3D11Query *begin_query;
    ID3D11Query *end_query;

    ID3D11Device *pDevice;
    ID3D11DeviceContext *pContext;

    uint64_t start_time, end_time, diff;
    int count_ = 1;
};

HRESULT InitDeviceAndCS();
HRESULT InitBuffer(UINT count, UINT LOOP_NUM);
HRESULT PerfKernel(const std::vector<UINT> &grid, DXTimer &timer);
void GetReport(UINT ThreadCount, UINT LOOP_NUM, DXTimer &timer);

HRESULT Run(UINT ThreadCount) {
    HRESULT hr = S_OK;

    UINT THREADS_PER_BLOCK = 32;
    UINT LOOP_NUM          = 100000;

    hr = InitBuffer(ThreadCount, LOOP_NUM);
    HR_CHECK(hr, "InitBuffer");

    DXTimer timer;

    std::vector<UINT> grid = {UINT(UP_DIV(ThreadCount, THREADS_PER_BLOCK))};

    hr = PerfKernel(grid, timer);
    HR_CHECK(hr, "PerfKernel");

    GetReport(ThreadCount, LOOP_NUM, timer);

    return hr;
}

int main(int argc, char **argv) {
    HRESULT hr = S_OK;
    hr = InitDeviceAndCS();
    HR_CHECK(hr, "Init");

    UINT cuda_cores = 1920;

    Run(128);
    Run(1024);
    Run(10240);
    Run(20480);

    Run(cuda_cores);
    Run(cuda_cores * 2);
    Run(cuda_cores * 4);
    Run(cuda_cores * 8);
    Run(cuda_cores * 16);

    return 0;
}

DXTimer::DXTimer() {
    this->pDevice  = g_device.get();
    this->pContext = g_context.get();
    disjoint       = nullptr;
    begin_query    = nullptr;
    end_query      = nullptr;
}

DXTimer::~DXTimer() {
    SAFE_RELEASE(disjoint);
    SAFE_RELEASE(begin_query);
    SAFE_RELEASE(end_query);
}

HRESULT DXTimer::Init(int count) {
    count_ = count;
    HRESULT hr = S_OK;

    D3D11_QUERY_DESC query_desc;
    query_desc.Query     = D3D11_QUERY_TIMESTAMP_DISJOINT;
    query_desc.MiscFlags = 0;

    hr = pDevice->CreateQuery(
        &query_desc,    // pQueryDesc
        &disjoint       // ppQuery
    );
    HR_CHECK(hr, "ID3D11Device::CreateQuery for DISJOINT");

    query_desc.Query = D3D11_QUERY_TIMESTAMP;
    hr = pDevice->CreateQuery(
        &query_desc,
        &begin_query
    );
    HR_CHECK(hr, "ID3D11Device::CreateQuery for TIMESTAMP");
    hr = pDevice->CreateQuery(
        &query_desc,
        &end_query
    );
    HR_CHECK(hr, "ID3D11Device::CreateQuery for TIMESTAMP");

    pContext->Begin(disjoint);

    return hr;
}

void DXTimer::Begin() {
    pContext->End(begin_query);
}

void DXTimer::End() {
    pContext->End(end_query);
}

float DXTimer::TimeEclapsed() {
    HRESULT hr = S_OK;

    pContext->End(disjoint);

    while (pContext->GetData(
        disjoint,   // pAsync
        nullptr,    // pData
        0,          // DataSize
        0           // GetDataFlags
        ) == S_FALSE) { Sleep(1); }

    D3D10_QUERY_DATA_TIMESTAMP_DISJOINT disjoint_value;
    hr = pContext->GetData(
        disjoint,
        &disjoint_value,
        sizeof(disjoint_value),
        0);
    if (disjoint_value.Disjoint) {
        printf("Error, still disjoint\n");
        return -1;
    }

    hr = pContext->GetData(begin_query,
        &start_time,
        sizeof(uint64_t),
        0);
    hr = pContext->GetData(end_query,
        &end_time,
        sizeof(uint64_t),
        0);

    diff = end_time - start_time;

    return diff / count_ / float(disjoint_value.Frequency) * 1000.0f;
}

HRESULT InitDeviceAndCS() {
    HRESULT hr = S_OK;

    ID3D11Device *pDevice         = nullptr;
    ID3D11DeviceContext *pContext = nullptr;
    D3D_FEATURE_LEVEL FeatureLevel;
    hr = D3D11CreateDevice(
        nullptr,                    // pAdapter
        D3D_DRIVER_TYPE_HARDWARE,   // DriverType
        nullptr,                    // Software
        0,                          // Flags
        nullptr,                    // pFeatureLevels
        0,                          // FeatureLevels
        D3D11_SDK_VERSION,          // SDKVersion
        &pDevice,                   // ppDevice
        &FeatureLevel,              // pFeatureLevel
        &pContext);                 // ppImmediateContext
    HR_CHECK(hr, "D3D11CreateDevice");

    g_device  = std::shared_ptr<ID3D11Device>(pDevice, [](ID3D11Device *p) { SAFE_RELEASE(p); });
    g_context = std::shared_ptr<ID3D11DeviceContext>(pContext, [](ID3D11DeviceContext *p) { SAFE_RELEASE(p); });

    ID3D11ComputeShader *pComputeShader;
    hr = pDevice->CreateComputeShader(
        g_fmla,                     // pShaderBytecode
        sizeof(g_fmla),             // BytecodeLength
        nullptr,                    // pClassLinkage
        &pComputeShader             // ppComputeShader
    );
    HR_CHECK(hr, "CreateComputeShader");

    g_fmla_cs = std::shared_ptr<ID3D11ComputeShader>(pComputeShader, [](ID3D11ComputeShader *p) { SAFE_RELEASE(p); });

    return hr;
}

std::shared_ptr<float> getInitData(int count) {
    static UINT gi  = 0;
    srand(11 + gi++);

    float *p = new float[count];
    for (int i = 0; i < count; ++i) {
        p[i] = float(rand() * 1.0 / RAND_MAX);
        // p[i] = float(rand() / (RAND_MAX / 4));
        // p[i] = float(0);
    }

    return std::shared_ptr<float>(p, [](float *p) { delete[] p; });
}

HRESULT createBuffer(std::shared_ptr<ID3D11Buffer> &spBuffer,
                     UINT count) {
    HRESULT hr = S_OK;

    ID3D11Buffer *pBuffer;
    D3D11_BUFFER_DESC bDesc;
    ZeroMemory(&bDesc, sizeof(bDesc));
    bDesc.ByteWidth      = count * sizeof(float);
    bDesc.Usage          = D3D11_USAGE_DEFAULT;
    bDesc.BindFlags      = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    bDesc.CPUAccessFlags = 0;
    bDesc.MiscFlags      = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    hr = g_device->CreateBuffer( 
        &bDesc,     // pDesc
        nullptr,    // pInitialData
        &pBuffer    // ppBuffer
    );
    HR_CHECK(hr, "CreateBuffer");

    spBuffer = std::shared_ptr<ID3D11Buffer>(pBuffer, [](ID3D11Buffer *p) { SAFE_RELEASE(p); });

    return hr;
}


template<typename T>
HRESULT createConstBuffer(std::shared_ptr<ID3D11Buffer> &spBuffer,
                          const T &host_value) {
    HRESULT hr = S_OK;

    D3D11_SUBRESOURCE_DATA init_data;
    init_data.pSysMem          = &host_value;
    init_data.SysMemPitch      = 0;
    init_data.SysMemSlicePitch = 0;

    D3D11_BUFFER_DESC constant_buffer_desc = {};
    constant_buffer_desc.ByteWidth         = ROUND_UP(sizeof(T), 16);
    constant_buffer_desc.Usage             = D3D11_USAGE_DEFAULT;
    constant_buffer_desc.BindFlags         = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_desc.CPUAccessFlags    = 0;

    ID3D11Buffer * pConstCuffer;
    hr = g_device->CreateBuffer(
        &constant_buffer_desc,
        &init_data,
        &pConstCuffer);
    HR_CHECK(hr, "CreateBuffer");

    spBuffer = std::shared_ptr<ID3D11Buffer>(pConstCuffer, [](ID3D11Buffer* p){ p->Release(); });

    return hr;
}

HRESULT writeBuffer(ID3D11Buffer *pBuffer,
                    const void *data) {
    HRESULT hr = S_OK;
    g_context->UpdateSubresource(
        pBuffer,        // pDstResource,
        0,              // DstSubresource,
        nullptr,        // pDstBox,
        data,           // pSrcData,
        0,              // SrcRowPitch,
        0               // SrcDepthPitch
    );
    HR_CHECK(hr, "UpdateSubresource");

    return S_OK;
}

HRESULT readBuffer(ID3D11Buffer *pBuffer,
                   void *data,
                   UINT byte_count) {
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC tmpDesc;
    ZeroMemory(&tmpDesc, sizeof(tmpDesc));
    ID3D11Buffer *tmpBuffer = nullptr;
    tmpDesc.ByteWidth       = byte_count;
    tmpDesc.Usage           = D3D11_USAGE_STAGING;
    tmpDesc.CPUAccessFlags  = D3D11_CPU_ACCESS_READ;
    hr = g_device->CreateBuffer(&tmpDesc, nullptr, &tmpBuffer);
    HR_CHECK(hr, "ID3D11Device::CreateBuffer");

    g_context->CopyResource(
        tmpBuffer,      // pDstResource
        pBuffer         // pSrcResource
    );
    HR_CHECK(hr, "ID3D11DeviceContext::CopyResource");

    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    hr = g_context->Map(
        tmpBuffer,          // pResource
        0,                  // Subresource
        D3D11_MAP_READ,     // MapType
        0,                  // MapFlags
        &mapped_resource    // pMappedResource
    );
    HR_CHECK(hr, "ID3D11DeviceContext::Map");

    memcpy(data, mapped_resource.pData, byte_count);

    g_context->Unmap(
        tmpBuffer,          // pResource,
        0                   // Subresource
    );
    HR_CHECK(hr, "ID3D11DeviceContext::Unmap");

    SAFE_RELEASE(tmpBuffer);

    return hr;
}

HRESULT getSRV(std::shared_ptr<ID3D11ShaderResourceView> &spBufferSRV,
               UINT count,
               ID3D11Buffer *pBuffer) {
    HRESULT hr = S_OK;

    D3D11_SHADER_RESOURCE_VIEW_DESC bSRVDesc;
    bSRVDesc.Format                = DXGI_FORMAT_R32_TYPELESS;
    bSRVDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;
    bSRVDesc.BufferEx.FirstElement = 0;
    bSRVDesc.BufferEx.NumElements  = count;
    bSRVDesc.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;

    ID3D11ShaderResourceView *pBufferSRV;
    hr = g_device->CreateShaderResourceView(
        pBuffer,        // pResource
        &bSRVDesc,      // pDesc
        &pBufferSRV     // ppSRView
    );
    HR_CHECK(hr, "CreateShaderResourceView");

    spBufferSRV = std::shared_ptr<ID3D11ShaderResourceView>(pBufferSRV, [](ID3D11ShaderResourceView *p) { SAFE_RELEASE(p); });

    return hr;
}

HRESULT getUAV(std::shared_ptr<ID3D11UnorderedAccessView> &spBufferUAV,
               UINT count,
               ID3D11Buffer *pBuffer) {
    HRESULT hr = S_OK;

    D3D11_UNORDERED_ACCESS_VIEW_DESC bUAVDesc;
    bUAVDesc.Format              = DXGI_FORMAT_R32_TYPELESS;
    bUAVDesc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
    bUAVDesc.Buffer.FirstElement = 0;
    bUAVDesc.Buffer.NumElements  = count;
    bUAVDesc.Buffer.Flags        = 0;
    bUAVDesc.Buffer.Flags        = D3D11_BUFFER_UAV_FLAG_RAW;
    ID3D11UnorderedAccessView *pBufferUAV;
    hr = g_device->CreateUnorderedAccessView(
        pBuffer,        // pResource
        &bUAVDesc,      // pDesc
        &pBufferUAV     // ppUAView
    );
    HR_CHECK(hr, "CreateUnorderedAccessView");

    spBufferUAV = std::shared_ptr<ID3D11UnorderedAccessView>(pBufferUAV, [](ID3D11UnorderedAccessView *p) { SAFE_RELEASE(p); });

    return hr;
}

HRESULT dispatchKernel(const std::vector<ID3D11ShaderResourceView*> &srvs,
                       const std::vector<ID3D11UnorderedAccessView*> &uavs,
                       const std::vector<ID3D11Buffer*> &cons,
                       ID3D11ComputeShader *pComputeShader,
                       const std::vector<UINT> &grid) {
    HRESULT hr = S_OK;

    g_context->CSSetShaderResources(
        0,                  // StartSlot
        UINT(srvs.size()),  // NumViews
        srvs.data()         // ppShaderResourceViews
    );
    HR_CHECK(hr, "ID3D11DeviceContext::CSSetShaderResources");

    g_context->CSSetUnorderedAccessViews(
        0,                  // StartSlot
        UINT(uavs.size()),  // NumUAVs
        uavs.data(),        // ppUnorderedAccessViews
        nullptr             // pUAVInitialCounts
    );
    HR_CHECK(hr, "ID3D11DeviceContext::CSSetUnorderedAccessViews");

    g_context->CSSetConstantBuffers(
        0,                  // StartSlot
        UINT(cons.size()),  // NumBuffers
        cons.data()         // ppConstantBuffers
    );
    HR_CHECK(hr, "ID3D11DeviceContext::CSSetConstantBuffers");

    g_context->CSSetShader(
        pComputeShader,     // pComputeShader
        nullptr,            // ppClassInstances
        0                   // NumClassInstances
    );
    HR_CHECK(hr, "ID3D11DeviceContext::CSSetShader");

    g_context->Dispatch(
        grid.size() > 0 ? grid[0] : 1,      // ThreadGroupCountX
        grid.size() > 1 ? grid[1] : 1,      // ThreadGroupCountY
        grid.size() > 2 ? grid[2] : 1       // ThreadGroupCountZ
    );
    HR_CHECK(hr, "ID3D11DeviceContext::Dispatch");

    std::vector<ID3D11ShaderResourceView *> null_srvs(srvs.size(), nullptr);
    g_context->CSSetShaderResources(
        0,
        UINT(srvs.size()),
        null_srvs.data()
    );
    std::vector<ID3D11UnorderedAccessView *> null_uavs(uavs.size(), nullptr);
    g_context->CSSetUnorderedAccessViews(
        0,
        UINT(uavs.size()),
        null_uavs.data(),
        nullptr
    );
    const std::vector<ID3D11Buffer*> null_cons(cons.size(), nullptr);
    g_context->CSSetConstantBuffers(
        0,
        UINT(null_cons.size()),
        null_cons.data()
    );

    return hr;
}

void GetReport(UINT ThreadCount, UINT LOOP_NUM, DXTimer &timer) {
    HRESULT hr = S_OK;
    hr = readBuffer(out_buffer.get(), out.get(), ThreadCount * sizeof(float));
    // for (int i = 0; i < 10; ++i) {
    //     printf("[%d] %f\n", i, out.get()[i]);
    // }

    float time_in_ms = timer.TimeEclapsed();
    double gflops    = 1.0 * ThreadCount * LOOP_NUM * 16 * 2 / time_in_ms / 1e6;
    printf(">> thread_count: %5d, time: %7.4f ms, gflops: %8.2f [Gemm Kernel]\n", ThreadCount, time_in_ms, gflops);
}

HRESULT InitBuffer(UINT count, UINT LOOP_NUM) {
    HRESULT hr = S_OK;

    typedef struct launch_param {
        DirectX::XMUINT4 dims;
    } launch_param_t;
    launch_param_t args{};
    args.dims = {LOOP_NUM, 0, 0, 0};
    hr = createConstBuffer<launch_param_t>(const_buffer, args);

    in  = getInitData(count);
    out = getInitData(count);

    hr = createBuffer(in_buffer, count);
    hr = writeBuffer(in_buffer.get(), in.get());
    hr = getSRV(in_srv, count, in_buffer.get());

    hr = createBuffer(out_buffer, count);
    hr = getUAV(out_uav, count, out_buffer.get());

    return hr;
}

HRESULT PerfKernel(const std::vector<UINT> &grid,
                   DXTimer &timer) {
    HRESULT hr = S_OK;

    std::vector<ID3D11ShaderResourceView*> srvs  = {in_srv.get()};
    std::vector<ID3D11UnorderedAccessView*> uavs = {out_uav.get()};
    std::vector<ID3D11Buffer*> cons              = {const_buffer.get()};

    int wc = 5;
    int ic = 10;

    hr = timer.Init(ic);

    for (int i = 0; i < wc; ++i) {
        hr = dispatchKernel(srvs, uavs, cons, g_fmla_cs.get(), grid);
    }

    timer.Begin();
    for (int i = 0; i < ic; ++i) {
        hr = dispatchKernel(srvs, uavs, cons, g_fmla_cs.get(), grid);
    }
    timer.End();

    return hr;
}
