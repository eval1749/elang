// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EMITTERS_OPCODES_X64_H_
#define ELANG_LIR_EMITTERS_OPCODES_X64_H_

namespace elang {
namespace lir {

// V0 opcode mnemonic
// V1 opcode mnemonic format
// V2 opcode mnemonic format1 format2
// V3 opcode mnemonic format1 format2 foramt3
//
//
// 0F = 2-byte escape
// 66 = Operand Size prefix
// F2 = REPNE prefix
// F3 = REP prefix
#define FOR_EACH_X64_OPCODE(V0, V1, V2, V3)  \
  FOR_EACH_X64_OPCODE_00_FF(V0, V1, V2, V3)  \
  FOR_EACH_X64_OPCODE_0F10(V0, V1, V2, V3)   \
  FOR_EACH_X64_OPCODE_0F38(V0, V1, V2, V3)   \
  FOR_EACH_X64_OPCODE_0F70(V0, V1, V2, V3)   \
  FOR_EACH_X64_OPCODE_0F80(V0, V1, V2, V3)   \
  FOR_EACH_X64_OPCODE_660F10(V0, V1, V2, V3) \
  FOR_EACH_X64_OPCODE_F30F10(V0, V1, V2, V3)

#define FOR_EACH_X64_OPCODE_00_FF(V0, V1, V2, V3) \
  /* 0x00 */                                      \
  V2(0x00, ADD, Eb, Gb)                           \
  V2(0x01, ADD, Ev, Gv)                           \
  V2(0x02, ADD, Gb, Eb)                           \
  V2(0x03, ADD, Gv, Ev)                           \
  V2(0x04, ADD, AL, Ib)                           \
  V2(0x05, ADD, eAX, Iz)                          \
  V1(0x06, PUSH, ES)                              \
  V1(0x07, POP, ES)                               \
  V2(0x08, OR, Eb, Gb)                            \
  V2(0x09, OR, Ev, Gv)                            \
  V2(0x0A, OR, Gb, Eb)                            \
  V2(0x0B, OR, Gv, Ev)                            \
  V2(0x0C, OR, AL, Ib)                            \
  V2(0x0D, OR, eAX, Iz)                           \
  V1(0x0E, PUSH, CS)                              \
  /* 0x10 */                                      \
  V2(0x10, ADC, Eb, Gb)                           \
  V2(0x11, ADC, Ev, Gv)                           \
  V2(0x12, ADC, Gb, Eb)                           \
  V2(0x13, ADC, Gv, Ev)                           \
  V2(0x14, ADC, AL, Ib)                           \
  V2(0x15, ADC, eAX, Iz)                          \
  V1(0x16, PUSH, SS)                              \
  V1(0x17, POP, SS)                               \
  V2(0x18, SBB, Eb, Gb)                           \
  V2(0x19, SBB, Ev, Gv)                           \
  V2(0x1A, SBB, Gb, Eb)                           \
  V2(0x1B, SBB, Gv, Ev)                           \
  V2(0x1C, SBB, AL, Ib)                           \
  V2(0x1D, SBB, eAX, Iz)                          \
  V1(0x1E, PUSH, DS)                              \
  V1(0x1F, POP, DS)                               \
  /* 0x20 */                                      \
  V2(0x20, AND, Eb, Gb)                           \
  V2(0x21, AND, Ev, Gv)                           \
  V2(0x22, AND, Gb, Eb)                           \
  V2(0x23, AND, Gv, Ev)                           \
  V2(0x24, AND, AL, Ib)                           \
  V2(0x25, AND, eAX, Iz)                          \
  V0(0x26, ES)                                    \
  V0(0x27, DAA)                                   \
  V2(0x28, SUB, Eb, Gb)                           \
  V2(0x29, SUB, Ev, Gv)                           \
  V2(0x2A, SUB, Gb, Eb)                           \
  V2(0x2B, SUB, Gv, Ev)                           \
  V2(0x2C, SUB, AL, Ib)                           \
  V2(0x2D, SUB, eAX, Iz)                          \
  V0(0x2E, CS)                                    \
  V0(0x2F, DAS)                                   \
  /* 0x30 */                                      \
  V2(0x30, XOR, Eb, Gb)                           \
  V2(0x31, XOR, Ev, Gv)                           \
  V2(0x32, XOR, Gb, Eb)                           \
  V2(0x33, XOR, Gv, Ev)                           \
  V2(0x34, XOR, AL, Ib)                           \
  V2(0x35, XOR, eAX, Iz)                          \
  V0(0x36, SS)                                    \
  V0(0x37, AAA)                                   \
  V2(0x38, CMP, Eb, Gb)                           \
  V2(0x39, CMP, Ev, Gv)                           \
  V2(0x3A, CMP, Gb, Eb)                           \
  V2(0x3B, CMP, Gv, Ev)                           \
  V2(0x3C, CMP, AL, Ib)                           \
  V2(0x3D, CMP, eAX, Iz)                          \
  V0(0x3E, DS)                                    \
  V0(0x3F, AAS)                                   \
  /* 0x40 */                                      \
  V1(0x40, INC, eAX)                              \
  V1(0x41, INC, eCX)                              \
  V1(0x42, INC, eDX)                              \
  V1(0x43, INC, eBX)                              \
  V1(0x44, INC, eSP)                              \
  V1(0x45, INC, eBP)                              \
  V1(0x46, INC, eSI)                              \
  V1(0x47, INC, eDI)                              \
  V1(0x48, DEC, eAX)                              \
  V1(0x49, DEC, eCX)                              \
  V1(0x4A, DEC, eDX)                              \
  V1(0x4B, DEC, eBX)                              \
  V1(0x4C, DEC, eSP)                              \
  V1(0x4D, DEC, eBP)                              \
  V1(0x4E, DEC, eSI)                              \
  V1(0x4F, DEC, eDI)                              \
  /* 0x50 */                                      \
  V1(0x50, PUSH, rAX)                             \
  V1(0x51, PUSH, rCX)                             \
  V1(0x52, PUSH, rDX)                             \
  V1(0x53, PUSH, rBX)                             \
  V1(0x54, PUSH, rSP)                             \
  V1(0x55, PUSH, rBP)                             \
  V1(0x56, PUSH, rSI)                             \
  V1(0x57, PUSH, rDI)                             \
  V1(0x58, POP, rAX)                              \
  V1(0x59, POP, rDX)                              \
  V1(0x5A, POP, rCX)                              \
  V1(0x5B, POP, rBX)                              \
  V1(0x5C, POP, rSP)                              \
  V1(0x5D, POP, rBP)                              \
  V1(0x5E, POP, rSI)                              \
  V1(0x5F, POP, rDI)                              \
  /* 0x60 */                                      \
  V0(0x60, PUSHAD)                                \
  V0(0x6660, PUSHA)                               \
  V0(0x61, POPAD)                                 \
  V0(0x6661, POPA)                                \
  V2(0x62, BOUND, Gv, Ma)                         \
  V2(0x63, MOVSXD, Gv, Ev)                        \
  V0(0x64, FS)                                    \
  V0(0x65, GS)                                    \
  V0(0x66, OPDSIZ)                                \
  V0(0x67, ADDRSIZ)                               \
  V1(0x68, PUSH, Iz)                              \
  V3(0x69, IMUL, Gv, Ev, Iz)                      \
  V1(0x6A, PUSH, Ib)                              \
  V3(0x6B, IMUL, Gv, Ev, Is)                      \
  /* 0x70 */                                      \
  V1(0x70, Jcc, Jb)                               \
  V1(0x70, JO, Jb)                                \
  V1(0x71, JNO, Jb)                               \
  V1(0x72, JB, Jb)                                \
  V1(0x73, JAE, Jb)                               \
  V1(0x74, JE, Jb)                                \
  V1(0x75, JNE, Jb)                               \
  V1(0x76, JBE, Jb)                               \
  V1(0x77, JA, Jb)                                \
  V1(0x78, JS, Jb)                                \
  V1(0x79, JNS, Jb)                               \
  V1(0x7A, JPE, Jb)                               \
  V1(0x7B, JPO, Jb)                               \
  V1(0x7C, JL, Jb)                                \
  V1(0x7D, JGE, Jb)                               \
  V1(0x7E, JLE, Jb)                               \
  V1(0x7F, JG, Jb)                                \
  /* 0x80 */                                      \
  V2(0x84, TEST, Eb, Gb)                          \
  V2(0x85, TEST, Ev, Gv)                          \
  V2(0x86, XCHG, Eb, Gb)                          \
  V2(0x87, XCHG, Ev, Gv)                          \
  V2(0x88, MOV, Eb, Gb)                           \
  V2(0x89, MOV, Ev, Gv)                           \
  V2(0x8A, MOV, Gb, Eb)                           \
  V2(0x8B, MOV, Gv, Ev)                           \
  V2(0x8C, MOV, Ew, Sw)                           \
  V2(0x8D, LEA, Gv, M)                            \
  V2(0x8E, MOV, Sw, Ew)                           \
  /* 0x90 */                                      \
  V0(0x90, NOP) /* XCHG eAX, eAX */               \
  V2(0x91, XCHG, eAX, eCX)                        \
  V2(0x92, XCHG, eAX, eDX)                        \
  V2(0x93, XCHG, eAX, eBX)                        \
  V2(0x94, XCHG, eAX, eSP)                        \
  V2(0x95, XCHG, eAX, eBP)                        \
  V2(0x96, XCHG, eAX, eSI)                        \
  V2(0x97, XCHG, eAX, eDI)                        \
  V0(0x6698, CBW)                                 \
  V0(0x98, CWDE)                                  \
  V0(0x99, CDQ)                                   \
  V0(0x6699, CWD)                                 \
  V1(0x9A, CALLF, Ap)                             \
  V0(0x9B, WAIT)                                  \
  V0(0x9C, PUSHFD)                                \
  V0(0x669C, PUSHF)                               \
  V0(0x9D, POPFD)                                 \
  V0(0x669D, POPF)                                \
  V0(0x9E, SAHF)                                  \
  V0(0x9F, LAHF)                                  \
  /* 0xA0 */                                      \
  V2(0xA0, MOV, AL, Ob)                           \
  V2(0xA1, MOV, eAX, Ov)                          \
  V2(0xA2, MOV, Ob, AL)                           \
  V2(0xA3, MOV, Ov, eAX)                          \
  V0(0xA4, MOVSB)                                 \
  V0(0xA5, MOVSD)                                 \
  V0(0x66A5, MOVSW)                               \
  V0(0x48A5, MOVSQ)                               \
  V0(0xA6, CMPSB)                                 \
  V0(0xA7, CMPSD)                                 \
  V0(0x66A7, CMPSW)                               \
  V2(0xA8, TEST, AL, Ib)                          \
  V2(0xA9, TEST, eAX, Iz)                         \
  V0(0xAA, STOSB)                                 \
  V0(0xAB, STOSD)                                 \
  V0(0x66AB, STOSW)                               \
  V0(0xAC, LODSB)                                 \
  V0(0xAD, LODSD)                                 \
  V0(0x66AD, LODSW)                               \
  V0(0xAE, SCASB)                                 \
  V0(0xAF, SCASD)                                 \
  V0(0x66AF, SCASW)                               \
  /* 0xB0 */                                      \
  V2(0xB0, MOV, AL, Ib)                           \
  V2(0xB1, MOV, CL, Ib)                           \
  V2(0xB2, MOV, DL, Ib)                           \
  V2(0xB3, MOV, BL, Ib)                           \
  V2(0xB4, MOV, AH, Ib)                           \
  V2(0xB5, MOV, CH, Ib)                           \
  V2(0xB6, MOV, DH, Ib)                           \
  V2(0xB7, MOV, BH, Ib)                           \
  V2(0xB8, MOV, eAX, Iv)                          \
  V2(0xB9, MOV, eCX, Iv)                          \
  V2(0xBA, MOV, eDX, Iv)                          \
  V2(0xBB, MOV, eBX, Iv)                          \
  V2(0xBC, MOV, eSP, Iv)                          \
  V2(0xBD, MOV, eBP, Iv)                          \
  V2(0xBE, MOV, eSI, Iv)                          \
  V2(0xBF, MOV, eDI, Iv)                          \
  /* 0xC0 */                                      \
  V1(0xC2, RET, Iw)                               \
  V0(0xC3, RET)                                   \
  V2(0xC4, LES, Gv, Mp)                           \
  V2(0xC5, LDS, Gv, Mp)                           \
  V2(0xC8, ENTER, Iw, Ib)                         \
  V0(0xC9, LEAVE)                                 \
  V1(0xCA, RETF, Iw)                              \
  V0(0xCB, RETF)                                  \
  V0(0xCC, INT3)                                  \
  V1(0xCD, INT, Ib)                               \
  V0(0xCE, INTO)                                  \
  V0(0xCF, IRET)                                  \
  /* 0xD0 */                                      \
  V0(0xD40A, AAM)                                 \
  V0(0xD50A, AAD)                                 \
  V0(0xD6, UNDEF)                                 \
  V0(0xD7, XLATB)                                 \
  V0(0xD7, XLAT)                                  \
  V0(0xD8, ESC_D8)                                \
  V0(0xD9, ESC_D9)                                \
  V0(0xDA, ESC_DA)                                \
  V0(0xDB, ESC_DB)                                \
  V0(0xDC, ESC_DC)                                \
  V0(0xDD, ESC_DD)                                \
  V0(0xDE, ESC_DE)                                \
  V0(0xDF, ESC_DF)                                \
                                                  \
  /* 0xE0 */                                      \
  V1(0xE0, LOOPNE, Jb) /* LOOPNZ */               \
  V1(0xE1, LOOPE, Jb)  /* LOOPZ */                \
  V1(0xE2, LOOP, Jb)                              \
  V1(0xE3, JECXZ, Jb)                             \
  V2(0xE4, IN, AL, Ib)                            \
  V2(0xE5, IN, eAX, Ib)                           \
  V2(0xE6, OUT, Ib, AL)                           \
  V2(0xE7, OUT, Ib, eAX)                          \
  V1(0xE8, CALL, Jv)                              \
  V1(0xE9, JMP, Jv)                               \
  V1(0xEA, JMP, Ap)                               \
  V1(0xEB, JMP, Jb)                               \
  V2(0xEC, IN, AL, DX)                            \
  V2(0xED, IN, eAX, DX)                           \
  V2(0xEE, OUT, DX, AL)                           \
  V2(0xEF, OUT, DX, eAX)                          \
  /* 0xF0 */                                      \
  V0(0xF0, LOCK)  /* prefix */                    \
  V0(0xF1, UD1)   /* UD1 (undocumented) */        \
  V0(0xF2, REPNE) /* prefix */                    \
  V0(0xF3, REP)   /* prefix */                    \
  V0(0xF390, PAUSE)                               \
  V0(0xF4, HLT)                                   \
  V0(0xF5, CMC)                                   \
  V0(0xF8, CLC)                                   \
  V0(0xF9, STC)                                   \
  V0(0xFA, CLI)                                   \
  V0(0xFB, STI)                                   \
  V0(0xFC, CLD)                                   \
  V0(0xFD, STD)

// Two byte opcode
#define FOR_EACH_X64_OPCODE_0F10(V0, V1, V2, V3) \
  V2(0x0F10, MOVUPS, Vps, Wps)                   \
  V2(0x0F11, MOVUPS, Wps, Vps)                   \
  V2(0x0F12, MOVLPS, Vq, Mq)                     \
  V2(0x0F13, MOVLPS, Mq, Vq)                     \
  V2(0x0F14, UNPCKLPS, Vps, Wq)                  \
  V2(0x0F15, UNPCKHPS, Vps, Wq)                  \
  V2(0x0F16, MOVHPS, Vq, Mq)                     \
  V2(0x0F17, MOVHPS, Mq, Vq)

#define FOR_EACH_X64_OPCODE_660F10(V0, V1, V2, V3) \
  V2(0x660F10, MOVUPD, Vps, Wps)                   \
  V2(0x660F11, MOVUPD, Wps, Vps)                   \
  V2(0x660F12, MOVLPD, Vq, Mq)                     \
  V2(0x660F13, MOVLPD, Mq, Vq)                     \
  V2(0x660F14, UNPCKLPD, Vpd, Wq)                  \
  V2(0x660F15, UNPCKHPD, Vpd, Wq)                  \
  V2(0x660F16, MOVHPD, Vq, Mq)                     \
  V2(0x660F17, MOVHPD, Mq, Vq)

#define FOR_EACH_X64_OPCODE_F30F10(V0, V1, V2, V3) \
  V2(0xF30F10, MOVSS, Vss, Wss)                    \
  V2(0xF30F11, MOVSS, Wss, Vss)                    \
  V2(0xF30F12, MOVSLDUP, Vq, Wq)                   \
  V2(0xF30F16, MOVSHDUP, Vq, Wq)                   \
                                                   \
  V2(0xF20F10, MOVSD, Vsd, Wsd)                    \
  V2(0xF20F11, MOVSD, Wsd, Vsd)                    \
  V2(0xF20F12, MOVDDUP, Vq, Wq)

#define FOR_EACH_X64_OPCODE_0F20(V0, V1, V2, V3)  \
  V2(0x0F28, MOVAPS, Vps, Wps)                    \
  V2(0x0F29, MOVAPS, Wps, Vps)                    \
  V2(0x0F2A, CVTPI2PS, Vps, Qq)                   \
  V2(0x0F2B, MOVNTPS, Mps, Vps)                   \
  V2(0x0F2C, CVTTPS2PI, Qq, Wps)                  \
  V2(0x0F2D, CVTPS2PI, Qq, Wps)                   \
  V2(0x0F2E, UCOMISS, Vss, Wss)                   \
  V2(0x0F2F, COMISS, Vss, Wss)                    \
                                                  \
  V2(0x660F28, MOVAPD, Vpd, Wpd)                  \
  V2(0x660F29, MOVAPD, Wpd, Vpd)                  \
  V2(0x660F2A, CVTPI2PD, Vpd, Qq)                 \
  V2(0x660F2B, MOVNTPD, Mpd, Vpd)                 \
  V2(0x660F2C, CVTTPD2PI, Qdq, Wpd)               \
  V2(0x660F2D, CVTPD2PI, Qdq, Wpd)                \
  V2(0x660F2E, UCOMISD, Vsd, Wsd)                 \
  V2(0x660F2F, COMISD, Vsd, Wsd)                  \
                                                  \
  V2(0xF20F2A, CVTSI2SD, Vsd, Ed)                 \
  V2(0xF20F2C, CVTTSD2SI, Gd, Wsd)                \
  V2(0xF20F2C, CVTSD2SI, Gd, Wsd)                 \
                                                  \
  V2(0xF30F2A, CVTSI2SS, Vss, Ed)                 \
  V2(0xF30F2C, CVTTSS2SI, Gd, Wss)                \
  V2(0xF20F2D, CVTSS2SI, Gd, Wss)                 \
                                                  \
  /* 0x0F40 */                                    \
  V2(0x0F40, CMOVcc, Gv, Ev)                      \
                                                  \
  V2(0x0F40, CMOVO, Gv, Ev) /* CMOVcc */          \
  V2(0x0F41, CMOVNO, Gv, Ev)                      \
  V2(0x0F42, CMOVB, Gv, Ev)   /* CMOVC CMOVNAE */ \
  V2(0x0F42, CMOVC, Gv, Ev)   /* CMOVC CMOVNAE */ \
  V2(0x0F42, CMOVHAE, Gv, Ev) /* CMOVC CMOVNAE */ \
  V2(0x0F43, CMOVAE, Gv, Ev)  /* CMOVNB CMOVNC */ \
  V2(0x0F43, CMOVNC, Gv, Ev)  /* CMOVNB CMOVNC */ \
  V2(0x0F43, CMOVNB, Gv, Ev)  /* CMOVNB CMOVNC */ \
  V2(0x0F44, CMOVE, Gv, Ev)   /* CMOVZ */         \
  V2(0x0F45, CMOVNE, Gv, Ev)  /* CMOVNZ */        \
  V2(0x0F46, CMOVBE, Gv, Ev)  /* CMOVNA */        \
  V2(0x0F47, CMOVA, Gv, Ev)   /* CMOVNBE */       \
  V2(0x0F48, CMOVS, Gv, Ev)                       \
  V2(0x0F49, CMOVNS, Gv, Ev)                      \
  V2(0x0F4A, CMOVPE, Gv, Ev) /* CMOVP */          \
  V2(0x0F4B, CMOVPO, Gv, Ev) /* CMOVNP */         \
  V2(0x0F4C, CMOVL, Gv, Ev)  /* CMOVNGE */        \
  V2(0x0F4D, CMOVGE, Gv, Ev) /* CMOVNL */         \
  V2(0x0F4E, CMOVLE, Gv, Ev) /* CMOVNG */         \
  V2(0x0F4F, CMOVG, Gv, Ev)  /* CMOVNLE */         \
                                                  \
  /* 0x0F50 */                                    \
  V2(0x0F50, MOVMSKPS, Gd, Ups)                   \
  V2(0x0F51, SQRTPS, Vps, Wps)                    \
  V2(0x0F52, RSQRTPS, Vps, Wps)                   \
  V2(0x0F53, RCPPS, Vps, Wps)                     \
  V2(0x0F54, ANDPS, Vps, Wps)                     \
  V2(0x0F55, ANDNPS, Vps, Wps)                    \
  V2(0x0F56, ORPS, Vps, Wps)                      \
  V2(0x0F57, XORPS, Vps, Wps)                     \
  V2(0x0F58, ADDPS, Vps, Wps)                     \
  V2(0x0F59, MULPS, Vps, Wps)                     \
  V2(0x0F5A, CVTPS2PD, Vpd, Wpd)                  \
  V2(0x0F5B, CVTDQ2PS, Vps, Wdq)                  \
  V2(0x0F5C, SUBPS, Vps, Wps)                     \
  V2(0x0F5D, MINPS, Vps, Wps)                     \
  V2(0x0F5E, DIVPS, Vps, Wps)                     \
  V2(0x0F5F, MAXPS, Vps, Wps)                     \
                                                  \
  V2(0x660F50, MOVMSKPD, Gd, Upd)                 \
  V2(0x660F51, SQRTPD, Vpd, Wpd)                  \
  V2(0x660F52, RSQRTPD, Vpd, Wpd)                 \
  V2(0x660F53, RCPPD, Vpd, Wpd)                   \
  V2(0x660F54, ANDPD, Vpd, Wpd)                   \
  V2(0x660F55, ANDNPD, Vpd, Wpd)                  \
  V2(0x660F56, ORPD, Vpd, Wpd)                    \
  V2(0x660F57, XORPD, Vpd, Wpd)                   \
  V2(0x660F58, ADDPD, Vpd, Wpd)                   \
  V2(0x660F59, MULPD, Vpd, Wpd)                   \
  V2(0x660F5A, CVTPD2PD, Vpd, Wpd)                \
  V2(0x660F5B, CVTDQ2PD, Vpd, Wdq)                \
  V2(0x660F5C, SUBPD, Vpd, Wpd)                   \
  V2(0x660F5D, MINPD, Vpd, Wpd)                   \
  V2(0x660F5E, DIVPD, Vpd, Wpd)                   \
  V2(0x660F5F, MAXPD, Vpd, Wpd)                   \
                                                  \
  V2(0xF30F51, SQRTSS, Vss, Wss)                  \
  V2(0xF30F58, ADDSS, Vss, Wss)                   \
  V2(0xF30F59, MULSS, Vss, Wss)                   \
  V2(0xF30F5A, CVTSS2SD, Vsd, Wss)                \
  V2(0xF30F5B, CVTDQ2SS, Vdq, Wps)                \
  V2(0xF30F5C, SUBSS, Vss, Wss)                   \
  V2(0xF30F5D, MINSS, Vss, Wss)                   \
  V2(0xF30F5E, DIVSS, Vss, Wss)                   \
  V2(0xF30F5F, MAXSS, Vss, Wss)                   \
                                                  \
  V2(0xF20F51, SQRTSD, Vsd, Wsd)                  \
  V2(0xF20F58, ADDSD, Vsd, Wsd)                   \
  V2(0xF20F59, MULSD, Vsd, Wsd)                   \
  V2(0xF20F5A, CVTSD2SS, Vss, Wsd)                \
  /* 0x5B */                                      \
  V2(0xF20F5C, SUBSD, Vsd, Wsd)                   \
  V2(0xF20F5D, MINSD, Vsd, Wsd)                   \
  V2(0xF20F5E, DIVSD, Vsd, Wsd)                   \
  V2(0xF20F5F, MAXSD, Vsd, Wsd)                   \
                                                  \
  /* 0x0F60 */                                    \
  V2(0x0F60, PUNPCKLBW, Pq, Qd) /* MMX */         \
  V2(0x0F61, PUNPCKLWD, Pq, Qd) /* MMX */         \
  V2(0x0F62, PUNPCKLDQ, Pq, Qd) /* MMX */         \
  V2(0x0F63, PCKSSWB, Pq, Qd)   /* MMX */         \
  V2(0x0F64, PCMPGTB, Pq, Qd)   /* MMX */         \
  V2(0x0F65, PCMPGTW, Pq, Qd)   /* MMX */         \
  V2(0x0F66, PCMPGTD, Pq, Qd)   /* MMX */         \
  V2(0x0F67, PACKUSWB, Pq, Qd)  /* MMX */         \
  V2(0x0F68, PUNPCKHBW, Pq, Qd) /* MMX */         \
  V2(0x0F69, PUNPCKHWD, Pq, Qd) /* MMX */         \
  V2(0x0F6A, PUNPCKHDQ, Pq, Qd) /* MMX */         \
  V2(0x0F6B, PACKSSDW, Pq, Qd)  /* MMX */         \
  V2(0x0F6E, MOVD, Pd, Ed)      /* MMX */         \
  V2(0x0F6F, MOVQ, Pq, Qq)      /* MMX */         \
                                                  \
  V2(0x660F60, PUNPCKLBW, Vdq, Wdq)  /* SSE */    \
  V2(0x660F61, PUNPCKLWD, Vdq, Wdq)  /* SSE */    \
  V2(0x660F62, PUNPCKLDQ, Vdq, Wdq)  /* SSE */    \
  V2(0x660F63, PCKSSWB, Vdq, Wdq)    /* SSE */    \
  V2(0x660F64, PCMPGTB, Vdq, Wdq)    /* SSE */    \
  V2(0x660F65, PCMPGTW, Vdq, Wdq)    /* SSE */    \
  V2(0x660F66, PCMPGTD, Vdq, Wdq)    /* SSE */    \
  V2(0x660F67, PACKUSWB, Vdq, Wdq)   /* SSE */    \
  V2(0x660F68, PUNPCKHBW, Vdq, Wdq)  /* SSE */    \
  V2(0x660F69, PUNPCKHWD, Vdq, Wdq)  /* SSE */    \
  V2(0x660F6A, PUNPCKHDQ, Vdq, Wdq)  /* SSE */    \
  V2(0x660F6B, PACKSSDW, Vdq, Wdq)   /* SSE */    \
  V2(0x660F6C, PUNPCKLQDQ, Vdq, Wdq) /* SSE */    \
  V2(0x660F6D, PUNPCKHQDQ, Vdq, Wdq) /* SSE */    \
  V2(0x660F6E, MOVD, Vdq, Ed)        /* SSE */    \
  V2(0x660F6F, MOVDQA, Vdq, Wdq)     /* SSE */    \
                                                  \
  V2(0xF30F6F, MOVDQU, Vdq, Wdq) /* SSE */

#define FOR_EACH_X64_OPCODE_0F70(V0, V1, V2, V3)    \
  V3(0x0F70, PSHUFW, Pq, Qq, Ib) /* MMX */          \
  /* 0x0F71x Grp 12 PSRLW, PSRAW, PSLLW */          \
  /* 0x0F72x Grp 13 PSRLD, PSRAD, PSLLD */          \
  /* 0x0F73x Grp 14 PSRLQ, PSRLDQ, PSLLQ, PSLLDQ */ \
  V2(0x0F74, PCMPEQB, Pq, Qq) /* MMX */             \
  V2(0x0F75, PCMPEQW, Pq, Qq) /* MMX */             \
  V2(0x0F76, PCMPEQD, Pq, Qq) /* MMX */             \
  V0(0x0F77, EMMS)            /* MMX */             \
  V2(0x0F78, VMREAD, Ed, Gd)  /* VMX */             \
  V2(0x0F79, VMWRITE, Gd, Ed) /* VMX */             \
                              /* 0x0F7A */          \
                              /* 0x0F7B */          \
                              /* 0x0F7C */          \
                              /* 0x0F7D */          \
  V2(0x0F7E, MOVD, Pd, Pd)    /* MMX */             \
  V2(0x0F7F, MOVQ, Qq, Pq)    /* MMX */             \
                                                    \
  V3(0x660F70, PSHUFD, Vdq, Wdq, Ib)                \
  V2(0x660F74, PCMPEQB, Vdq, Wdq)                   \
  V2(0x660F75, PCMPEQW, Vdq, Wdq)                   \
  V2(0x660F76, PCMPEQD, Vdq, Wdq)                   \
  /* 0x660F77 */                                    \
  /* 0x660F78 */                                    \
  /* 0x660F79 */                                    \
  /* 0x660F7A */                                    \
  /* 0x660F7B */                                    \
  V2(0x660F7C, HADDPD, Vpd, Wpd)                    \
  V2(0x660F7D, HSUBPD, Vpd, Wpd)                    \
  V2(0x660F7E, MOVD, Ed, Vdq)                       \
  V2(0x660F7F, MOVDQA, Wdq, Vdq)                    \
                                                    \
  V3(0xF20F70, PSHUFHW, Vdq, Wdq, Ib)               \
  V2(0xF20F7C, HADDPS, Vps, Wps)                    \
  V2(0xF20F7D, HSUBPS, Vps, Wps)                    \
                                                    \
  V3(0xF30F70, PSHUFLW, Vdq, Wdq, Ib)               \
  V2(0xF30F7E, MOVQ, Vq, Wq)                        \
  V2(0xF30F7F, MOVDQU, Wdq, Vdq)

#define FOR_EACH_X64_OPCODE_0F80(V0, V1, V2, V3) \
  V1(0x0F80, Jcc, Jv)                            \
  V1(0x0F80, JO, Jv)                             \
  V1(0x0F81, JNO, Jv)                            \
  V1(0x0F82, JB, Jv)  /* JC JNAE */              \
  V1(0x0F83, JAE, Jv) /* JNB JNC */              \
  V1(0x0F84, JE, Jv)  /* JZ */                   \
  V1(0x0F85, JNE, Jv) /* JNZ */                  \
  V1(0x0F86, JBE, Jv) /* JNA */                  \
  V1(0x0F87, JA, Jv)  /* JNBE */                 \
  V1(0x0F88, JS, Jv)                             \
  V1(0x0F89, JNS, Jv)                            \
  V1(0x0F8A, JPE, Jv) /* JP */                   \
  V1(0x0F8B, JPO, Jv) /* JNP */                  \
  V1(0x0F8C, JL, Jv)  /* JNGE */                 \
  V1(0x0F8D, JGE, Jv) /* JNL */                  \
  V1(0x0F8E, JLE, Jv) /* JNG */                  \
  V1(0x0F8F, JG, Jv)  /* JNLE */                  \
                                                 \
  /* 0x0FA2 */                                   \
  V0(0x0FA2, CPUID)                              \
  /* 0x0FB0 */                                   \
  V2(0x0FB6, MOVZX, Gv, Eb)                      \
  V2(0x0FB7, MOVZX, Gv, Ew)                      \
  V2(0x0FBE, MOVSX, Gv, Eb)                      \
  V2(0x0FBF, MOVSX, Gv, Ew)                      \
  /* 0x0FC0 */                                   \
  V2(0x0FC0, XADD, Eb, Gb)                       \
  V2(0x0FC1, XADD, Ev, Gv)                       \
  V3(0x0FC2, CMPSS, Vss, Wss, Ib)                \
  V2(0x0FC3, MOVNTI, Md, Gd)                     \
  V3(0x0FC4, PINSRW, Pq, Ew, Ib)                 \
  V3(0x0FC5, PEXTRW, Gd, Nq, Ib)                 \
  V3(0x0FC6, SHUFPS, Pq, Ew, Ib)                 \
  /* 0x0FC7 Grp 9 */                             \
  V3(0xF30FC2, CMPPS, Vps, Wps, Ib)              \
  V3(0x660FC2, CMPPD, Vpd, Wpd, Ib)              \
  V3(0xF20FC2, CMPSD, Vsd, Wsd, Ib)              \
                                                 \
  /* 0x0FD0 */                                   \
  V2(0x660FD0, ADDSUBPD, Vpd, Wpd) /* SSE3 */    \
  V2(0x660FD1, PSRLW, Vdq, Wdq)    /* SSE2 */    \
  V2(0x660FD2, PSRLD, Vdq, Wdq)    /* SSE2 */    \
  V2(0x660FD3, PSRLQ, Vdq, Wdq)    /* SSE2 */    \
  V2(0x660FD4, PADDQ, Vdq, Wdq)    /* SSE2 */    \
  V2(0x660FD5, PMULLW, Vdq, Wdq)   /* SSE2 */    \
  V2(0x660FD6, MOVQ, Wq, Vq)                     \
  V2(0x660FD7, PMOVMKSB, Gd, Udq)                \
  V2(0x660FD8, PSUBUSB, Vdq, Wdq)                \
  V2(0x660FD9, PSUBUSW, Vdq, Wdq)                \
  V2(0x660FDA, PMINUB, Vdq, Wdq)                 \
  V2(0x660FDB, PAND, Vdq, Wdq)                   \
  V2(0x660FDC, PADDSUB, Vdq, Wdq)                \
  V2(0x660FDD, PADDUBW, Vdq, Wdq)                \
  V2(0x660FDE, PMAXUB, Vdq, Wdq)                 \
  V2(0x660FDF, PANDN, Vdq, Wdq)                  \
                                                 \
  /* 0x0FE0 */                                   \
  V2(0x660FE0, PAVGB, Vdq, Wdq)                  \
  V2(0x660FE1, PSRAW, Vdq, Wdq)                  \
  V2(0x660FE2, PSRAD, Vdq, Wdq)                  \
  V2(0x660FE3, PAVGW, Vdq, Wdq)                  \
  V2(0x660FE4, PMULHUW, Vdq, Wdq)                \
  V2(0x660FE5, PMULHW, Vdq, Wdq)                 \
  V2(0x660FE6, CVTTPD2DQ, Vdq, Wdq)              \
  V2(0x660FE7, MOVNTDQ, Mdq, Wdq)                \
  V2(0x660FE8, PSUBSB, Vdq, Wdq)                 \
  V2(0x660FE9, PSUBSW, Vdq, Wdq)                 \
  V2(0x660FEA, PMINSW, Vdq, Wdq)                 \
  V2(0x660FEB, POR, Vdq, Wdq)                    \
  V2(0x660FEC, PADDSB, Vdq, Wdq)                 \
  V2(0x660FED, PADDSW, Vdq, Wdq)                 \
  V2(0x660FEE, PMAXSW, Vdq, Wdq)                 \
  V2(0x660FEF, PXOR, Vdq, Wdq)                   \
  /* 0x0FF0 */                                   \
  /* 0x660FF0 */                                 \
  V2(0x660FF1, PSLLW, Vdq, Wdq)                  \
  V2(0x660FF2, PSLLD, Vdq, Wdq)                  \
  V2(0x660FF3, PSLLQ, Vdq, Wdq)                  \
  V2(0x660FF4, PMULUDQ, Vdq, Wdq)                \
  V2(0x660FF5, PMADDWD, Vdq, Wdq)                \
  V2(0x660FF6, PSADBW, Vdq, Wdq)                 \
  V2(0x660FF7, MASKMOVDQU, Mdq, Wdq)             \
  V2(0x660FF8, PSUBB, Vdq, Wdq)                  \
  V2(0x660FF9, PSUBW, Vdq, Wdq)                  \
  V2(0x660FFA, PSUBD, Vdq, Wdq)                  \
  V2(0x660FFB, PSUBQ, Vdq, Wdq)                  \
  V2(0x660FFC, PADDB, Vdq, Wdq)                  \
  V2(0x660FFD, PADDW, Vdq, Wdq)                  \
  V2(0x660FFE, PADD, Vdq, Wdq)

// Three-byte Opcode (First Two Bytes are 0F 38)
#define FOR_EACH_X64_OPCODE_0F38(V0, V1, V2, V3) \
  V2(0x660F3800, PSHUFB, Vdq, Wdq)               \
  V2(0x660F3801, PHADDW, Vdq, Wdq)               \
  V2(0x660F3802, PHADDD, Vdq, Wdq)               \
  V2(0x660F3803, PHADDSW, Vdq, Wdq)              \
  V2(0x660F3804, PMADDSUBSW, Vdq, Wdq)           \
  V2(0x660F3805, PHSUBW, Vdq, Wdq)               \
  V2(0x660F3806, PHSUBD, Vdq, Wdq)               \
  V2(0x660F3807, PHSUBSW, Vdq, Wdq)              \
  V2(0x660F3808, PSIGNB, Vdq, Wdq)               \
  V2(0x660F3809, PSIGNW, Vdq, Wdq)               \
  V2(0x660F380A, PSIGND, Vdq, Wdq)               \
  V2(0x660F380B, PMULHRSW, Vdq, Wdq)             \
  V2(0x660F381C, PABSB, Vdq, Wdq)                \
  V2(0x660F381D, PABSW, Vdq, Wdq)                \
  V2(0x660F381E, PABSD, Vdq, Wdq)                \
  V2(0x48F20F2C, CVTTSD2SI, Gq, Wsd)             \
  V2(0x48F20F2C, CVTSD2SI, Gq, Wsd)              \
  V2(0x48F20F2A, CVTSI2SD, Vsd, Eq)              \
  V2(0x48F30F2A, CVTSI2SS, Vss, Eq)              \
  V2(0x48F30F2C, CVTTSS2SI, Gq, Wss)             \
  V2(0x48F20F2D, CVTSS2SI, Gq, Wss)              \
  V2(0x48660F6E, MOVQ, Vdq, Eq)                  \
  V2(0x48660F7E, MOVQ, Eq, Vdq)

// VX opcode {TwoByte, ModRm}
#define FOR_EACH_X64_OPCODE_EXTEND(VX)          \
  VX(0x0F, TwoByte)                             \
  VX(0x0F38, ModRm)                             \
  VX(0x80, ModRm)                               \
  VX(0x81, ModRm)                               \
  VX(0x82, ModRm)                               \
  VX(0x83, ModRm)                               \
  VX(0x8F, ModRm)                               \
  VX(0xC0, ModRm) /* Grp2, Eb, Ib */            \
  VX(0xC1, ModRm) /* Grp2, Ev, Ib */            \
  VX(0xC6, ModRm) /* Grp11, Eb, Ib MOV_Eb_Ib */ \
  VX(0xC7, ModRm) /* Grp11, Ev, Iv MOV_Ev_Iz */ \
  VX(0xD0, ModRm) /* Grp2, Eb, 1 */             \
  VX(0xD1, ModRm) /* Grp2, Ev, 1 */             \
  VX(0xD2, ModRm) /* Grp2, Eb, CL */            \
  VX(0xD3, ModRm) /* Grp2, Ev, CL */            \
  VX(0xF6, ModRm)                               \
  VX(0xF7, ModRm)                               \
  VX(0xFE, ModRm) /* Group 4 */                 \
  VX(0xFF, ModRm) /* Group 5 */                 \
  VX(0x660F71, ModRm)                           \
  VX(0x660F72, ModRm)                           \
  VX(0x660F73, ModRm)

#define FOR_EACH_X64_OPEXT(VX1, VX2)                    \
  VX1(0x8F, 0, POP, Ev)                                 \
  /* Group 1 */                                         \
  VX2(0x80, 0, ADD, Eb, Ib)                             \
  VX2(0x80, 1, OR, Eb, Ib)                              \
  VX2(0x80, 2, ADC, Eb, Ib)                             \
  VX2(0x80, 3, SBB, Eb, Ib)                             \
  VX2(0x80, 4, AND, Eb, Ib)                             \
  VX2(0x80, 5, SUB, Eb, Ib)                             \
  VX2(0x80, 6, XOR, Eb, Ib)                             \
  VX2(0x80, 7, CMP, Eb, Ib)                             \
                                                        \
  VX2(0x81, 0, ADD, Ev, Iz)                             \
  VX2(0x81, 1, OR, Ev, Iz)                              \
  VX2(0x81, 2, ADC, Ev, Iz)                             \
  VX2(0x81, 3, SBB, Ev, Iz)                             \
  VX2(0x81, 4, AND, Ev, Iz)                             \
  VX2(0x81, 5, SUB, Ev, Iz)                             \
  VX2(0x81, 6, XOR, Ev, Iz)                             \
  VX2(0x81, 7, CMP, Ev, Iz)                             \
                                                        \
  VX2(0x83, 0, ADD, Ev, Ib)                             \
  VX2(0x83, 1, OR, Ev, Ib)                              \
  VX2(0x83, 2, ADC, Ev, Ib)                             \
  VX2(0x83, 3, SBB, Ev, Ib)                             \
  VX2(0x83, 4, AND, Ev, Ib)                             \
  VX2(0x83, 5, SUB, Ev, Ib)                             \
  VX2(0x83, 6, XOR, Ev, Ib)                             \
  VX2(0x83, 7, CMP, Ev, Ib)                             \
                                                        \
  VX2(0xC0, 0, ROL, Eb, Ib)                             \
  VX2(0xC0, 1, ROR, Eb, Ib)                             \
  VX2(0xC0, 2, RCL, Eb, Ib)                             \
  VX2(0xC0, 3, RCR, Eb, Ib)                             \
  VX2(0xC0, 4, SHL, Eb, Ib)                             \
  VX2(0xC0, 5, SHR, Eb, Ib)                             \
  /* 6 */                                               \
  VX2(0xC0, 7, SAR, Eb, Ib)                             \
                                                        \
  VX2(0xC1, 0, ROL, Ev, Ib)                             \
  VX2(0xC1, 1, ROR, Ev, Ib)                             \
  VX2(0xC1, 2, RCL, Ev, Ib)                             \
  VX2(0xC1, 3, RCR, Ev, Ib)                             \
  VX2(0xC1, 4, SHL, Ev, Ib)                             \
  VX2(0xC1, 5, SHR, Ev, Ib)                             \
  VX2(0xC1, 7, SAR, Ev, Ib)                             \
                                                        \
  VX2(0xC6, 0, MOV, Eb, Ib)                             \
  VX2(0xC7, 0, MOV, Ev, Iz)                             \
                                                        \
  VX2(0xD0, 0, ROL, Eb, 1)                              \
  VX2(0xD0, 1, ROR, Eb, 1)                              \
  VX2(0xD0, 2, RCL, Eb, 1)                              \
  VX2(0xD0, 3, RCR, Eb, 1)                              \
  VX2(0xD0, 4, SHL, Eb, 1)                              \
  VX2(0xD0, 5, SHR, Eb, 1)                              \
                                                        \
  VX2(0xD0, 6, SAR, Eb, 1)                              \
  VX2(0xD1, 0, ROL, Ev, 1)                              \
  VX2(0xD1, 1, ROR, Ev, 1)                              \
  VX2(0xD1, 2, RCL, Ev, 1)                              \
  VX2(0xD1, 3, RCR, Ev, 1)                              \
  VX2(0xD1, 4, SHL, Ev, 1)                              \
  VX2(0xD1, 5, SHR, Ev, 1)                              \
  VX2(0xD1, 7, SAR, Ev, 1)                              \
                                                        \
  VX2(0xD2, 0, ROL, Eb, CL)                             \
  VX2(0xD2, 1, ROR, Eb, CL)                             \
  VX2(0xD2, 2, RCL, Eb, CL)                             \
  VX2(0xD2, 3, RCR, Eb, CL)                             \
  VX2(0xD2, 4, SHL, Eb, CL)                             \
  VX2(0xD2, 5, SHR, Eb, CL)                             \
  VX2(0xD2, 7, SAR, Eb, CL)                             \
                                                        \
  VX2(0xD3, 0, ROL, Ev, CL)                             \
  VX2(0xD3, 1, ROR, Ev, CL)                             \
  VX2(0xD3, 2, RCL, Ev, CL)                             \
  VX2(0xD3, 3, RCR, Ev, CL)                             \
  VX2(0xD3, 4, SHL, Ev, CL)                             \
  VX2(0xD3, 5, SHR, Ev, CL)                             \
  VX2(0xD3, 7, SAR, Ev, CL)                             \
  /* Group 3 - F6, F7 */                                \
  VX2(0xF6, 0, TEST, Eb, Ib)                            \
  VX1(0xF6, 2, NOT, Eb)                                 \
  VX1(0xF6, 3, NEG, Eb)                                 \
  VX1(0xF6, 4, MUL, Eb)                                 \
  VX1(0xF6, 5, IMUL, Eb)                                \
  VX1(0xF6, 6, DIV, Eb)                                 \
  VX1(0xF6, 7, IDIV, Eb)                                \
                                                        \
  VX2(0xF7, 0, TEST, Ev, Iz)                            \
  VX1(0xF7, 2, NOT, Ev)                                 \
  VX1(0xF7, 3, NEG, Ev)                                 \
  VX1(0xF7, 4, MUL, Ev)  /* vDX:eAX = eAX * Ev */       \
  VX1(0xF7, 5, IMUL, Ev) /* vDX:eAX = eAX * Ev  */      \
  VX1(0xF7, 6, DIV, Ev)  /* eAX, vDX = vDX:eAX / Ev  */ \
  VX1(0xF7, 7, IDIV, Ev) /* eAX, vDX = vDX:eAX / Ev */  \
                                                        \
  /* Group 4 */                                         \
  VX1(0xFE, 0, INC, Eb)                                 \
  VX1(0xFE, 1, DEC, Eb)                                 \
                                                        \
  /* Group 5 0xFF */                                    \
  VX1(0xFF, 0, INC, Ev)                                 \
  VX1(0xFF, 1, DEC, Ev)                                 \
  VX1(0xFF, 2, CALL, EvF64)                             \
  VX1(0xFF, 3, CALLF, Ev)                               \
  VX1(0xFF, 4, JMP, EvF64)                              \
  VX1(0xFF, 5, JMPF, Ev)                                \
  VX1(0xFF, 6, PUSH, EvD64)                             \
  /* Group 12 */                                        \
  VX2(0x660F71, 2, PSRLW, Udq, Ib)                      \
  VX2(0x660F71, 4, PSRAW, Udq, Ib)                      \
  VX2(0x660F71, 6, PSLLW, Udq, Ib)                      \
  /* Group 13 */                                        \
  VX2(0x660F72, 2, PSRLD, Udq, Ib)                      \
  VX2(0x660F72, 4, PSRAD, Udq, Ib)                      \
  VX2(0x660F72, 6, PSLLD, Udq, Ib)                      \
  /* Group 14 */                                        \
  VX2(0x660F73, 2, PSRLQ, Udq, Ib)                      \
  VX2(0x660F73, 3, PSRLDQ, Udq, Ib)                     \
  VX2(0x660F73, 6, PSLLQ, Udq, Ib)                      \
  VX2(0x660F73, 7, PSLLDQ, Udq, Ib)

namespace isa {

enum class Opcode {
#define V0(opcode, mnemonic) mnemonic = opcode,
#define V1(opcode, mnemonic, format) mnemonic##_##format = opcode,
#define V2(opcode, mnemonic, format1, format2) \
  mnemonic##_##format1##_##format2 = opcode,
#define V3(opcode, mnemonic, format1, format2, format3) \
  mnemonic##_##format1##_##format2##_##format3 = opcode,
  FOR_EACH_X64_OPCODE(V0, V1, V2, V3)
#undef V0
#undef V1
#undef V2
#undef V3

#define VX1(opcode, opext, mnemonic, format) mnemonic##_##format = opcode,
#define VX2(opcode, opext, mnemonic, format1, format2) \
  mnemonic##_##format1##_##format2 = opcode,
      FOR_EACH_X64_OPEXT(VX1, VX2)
#undef VX1
#undef VX2
};

// OpcodeExt is used in r/m field of ModRm.
enum class OpcodeExt {
#define VX1(opcode, opext, mnemonic, format) mnemonic##_##format = opext,
#define VX2(opcode, opext, mnemonic, format1, format2) \
  mnemonic##_##format1##_##format2 = opext,
  FOR_EACH_X64_OPEXT(VX1, VX2)
#undef VX1
#undef VX2
};

}  // namespace isa
}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EMITTERS_OPCODES_X64_H_
