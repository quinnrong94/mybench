
set CUR_DIR=%~dp0

set fname=FMLA.hlsl

fxc %CUR_DIR%\%fname% ^
    /E main ^
    /Vn g_fmla ^
    /T cs_5_0 ^
    /Fh
