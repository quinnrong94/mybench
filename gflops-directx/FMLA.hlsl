ByteAddressBuffer in_buf: register(t0);
RWByteAddressBuffer out_buf: register(u0);

cbuffer Shapes: register(b0)
{
    vector<uint, 4> dims;
};

#define ReadFloat(_buf, index) asfloat(_buf.Load((index) * 4))
#define WriteFloat(_buf, index, value) _buf.Store((index) * 4, asuint(value))

#define THREADS_PER_BLOCK 32

[numthreads(THREADS_PER_BLOCK, 1, 1)]
void main(uint groupIndex : SV_GroupIndex) {
    float v0 = ReadFloat(in_buf, groupIndex.x);
    for (uint i = 0; i < dims[0]; ++i) {
        v0 = v0 * v0 + v0;
        v0 = v0 * v0 + v0;
        v0 = v0 * v0 + v0;
        v0 = v0 * v0 + v0;

        v0 = v0 * v0 + v0;
        v0 = v0 * v0 + v0;
        v0 = v0 * v0 + v0;
        v0 = v0 * v0 + v0;

        v0 = v0 * v0 + v0;
        v0 = v0 * v0 + v0;
        v0 = v0 * v0 + v0;
        v0 = v0 * v0 + v0;

        v0 = v0 * v0 + v0;
        v0 = v0 * v0 + v0;
        v0 = v0 * v0 + v0;
        v0 = v0 * v0 + v0;
    }
    WriteFloat(out_buf, groupIndex.x, v0);
}
