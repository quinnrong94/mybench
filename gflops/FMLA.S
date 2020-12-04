#ifdef __aarch64__

.text
.align 5
#ifdef __APPLE__
.global _FmlaLoop12

_FmlaLoop12:
#else
.global FmlaLoop12

FmlaLoop12:
#endif
.loop1:
    fmla v0.4s, v0.4s, v0.4s
    fmla v1.4s, v1.4s, v1.4s
    fmla v2.4s, v2.4s, v2.4s
    fmla v3.4s, v3.4s, v3.4s

    fmla v4.4s, v4.4s, v4.4s
    fmla v5.4s, v5.4s, v5.4s
    fmla v6.4s, v6.4s, v6.4s
    fmla v7.4s, v7.4s, v7.4s

    fmla v8.4s, v8.4s, v8.4s
    fmla v9.4s, v9.4s, v9.4s
    fmla v10.4s, v10.4s, v10.4s
    fmla v11.4s, v11.4s, v11.4s

    subs x0, x0, #1
    bne .loop1
    ret

#else
#ifdef __arm__

.text
.align 5
#ifdef __APPLE__
.global _FmlaLoop12

_FmlaLoop12:
#else
.global FmlaLoop12

FmlaLoop12:
#endif
push {lr}

.loop1:
    vmla.f32 q0, q0, q0
    vmla.f32 q1, q1, q1
    vmla.f32 q2, q2, q2
    vmla.f32 q3, q3, q3

    vmla.f32 q4, q4, q4
    vmla.f32 q5, q5, q5
    vmla.f32 q6, q6, q6
    vmla.f32 q7, q7, q7

    vmla.f32 q8, q8, q8
    vmla.f32 q9, q9, q9
    vmla.f32 q10, q10, q10
    vmla.f32 q11, q11, q11

    subs r0, r0, #1
    bne .loop1

pop {pc}

#endif
#endif
