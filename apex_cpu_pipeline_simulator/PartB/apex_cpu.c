/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"
#include "apex_macros.h"

int memEmpty = 0;
int exeEmpty = 0;
int memReg;
int memRes;
int exeReg , exeRes;
/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_ADDL:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }
        case OPCODE_SUB:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_SUBL:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }
        case OPCODE_MUL:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_DIV:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_AND:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_OR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }

        case OPCODE_LOAD:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_LDR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_STORE:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }

        case OPCODE_STR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->rs3);
            break;
        }

        case OPCODE_BZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_BNZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_HALT:
        {
            printf("%s", stage->opcode_str);
            break;
        }

        case OPCODE_CMP:
        {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
            break;
        }

        case OPCODE_NOP:
        {
            printf("%s ", stage->opcode_str);
            break;
        }
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->fetch.has_insn)
    {
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;

        /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        cpu->fetch.rs1 = current_ins->rs1;
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.rs3 = current_ins->rs3;
        cpu->fetch.imm = current_ins->imm;

        if(cpu->decode.stalled == 0)
        {
            /* Update PC for next instruction */
            cpu->pc += 4;

            /* Copy data from fetch latch to decode latch*/
            cpu->decode = cpu->fetch;

        }
        else
        {
            (cpu->fetch.stalled = 1);
        }

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Fetch", &cpu->fetch);
        }

        /* Stop fetching new instructions if HALT is fetched */
        //if (cpu->fetch.opcode == OPCODE_HALT && cpu->decode.stalled == 0)
       // {
         //   //printf("Halt detected\n");
           // cpu->decode = cpu->fetch;
            //cpu->fetch.has_insn = FALSE;
       // }
    }

    else if(cpu->decode.stalled == 0 && cpu->fetch.opcode != OPCODE_HALT)
    {
 		/* Update PC for next instruction */
            //cpu->pc += 4;

            /* Copy data from fetch latch to decode latch*/
            cpu->decode = cpu->fetch;
            //cpu->fetch.stalled = 0;
 	   }
    else
    {
        cpu->fetch.has_insn = FALSE;
        printf("fetch   :   EMPTY\n");
    }

}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
    if (cpu->decode.has_insn && cpu->decode.stalled == 0)
    {
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {

            case OPCODE_ADD:
            {
              /*
              if(cpu->valid_regs[cpu->decode.rs1] == 0 && cpu->valid_regs[cpu->decode.rs2] == 0)
              {
                  cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                  cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                  cpu->valid_regs[cpu->decode.rd] = 1;
              }
              else if(cpu->valid_regs[cpu->decode.rs1] == 0 && cpu->valid_regs[cpu->decode.rs2] != 0)
              {
                if(cpu->decode.rs2 == cpu->execute.rd)
                {
                  cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                  cpu->decode.rs2_value = cpu->execute.result_buffer;
                  cpu->valid_regs[cpu->decode.rd] = 1;
                }
                else if(cpu->decode.rs2 == memReg)
                {
                  cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                  cpu->decode.rs2_value = memRes;
                  cpu->valid_regs[cpu->decode.rd] = 1;
                }
                else
                {
                  cpu->decode.stalled = 1;
                  cpu->fetch.stalled = 1;
                }
              }
              else if(cpu->valid_regs[cpu->decode.rs2] == 0 && cpu->valid_regs[cpu->decode.rs1] != 0)
              {
                if(cpu->decode.rs1 == cpu->execute.rd)
                {
                  cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                  cpu->decode.rs1_value = cpu->execute.result_buffer;
                  cpu->valid_regs[cpu->decode.rd] = 1;
                }
                else if(cpu->decode.rs1 == memReg)
                {
                  cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                  cpu->decode.rs1_value = memRes;
                  cpu->valid_regs[cpu->decode.rd] = 1;
                }
                else
                {
                  cpu->decode.stalled = 1;
                  cpu->fetch.stalled = 1;
                }
              }
              else if(cpu->valid_regs[cpu->decode.rs1] != 0 && cpu->valid_regs[cpu->decode.rs2] != 0 && cpu->decode.rs2 == cpu->decode.rs1)
              {
                if(cpu->decode.rs1 == cpu->execute.rd)
                {
                  cpu->decode.rs2_value = cpu->execute.result_buffer;
                  cpu->decode.rs1_value = cpu->execute.result_buffer;
                  cpu->valid_regs[cpu->decode.rd] = 1;
                }
                else if(cpu->decode.rs1 == memReg)
                {
                  cpu->decode.rs2_value = memRes;
                  cpu->decode.rs1_value = memRes;
                  cpu->valid_regs[cpu->decode.rd] = 1;
                }
                else
                {
                  cpu->decode.stalled = 1;
                  cpu->fetch.stalled = 1;
                }
              }

*/
              int rs1_val , rs2_val = 0;

              if(cpu->valid_regs[cpu->decode.rs1] == 0)
              {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == cpu->execute.rd)
              {
                //printf("Execute RD = R%d\n", cpu->execute.rd);
                if(cpu->valid_regs[cpu->execute.rd] == 0)
                {
                  cpu->decode.rs1_value = cpu->regs[cpu->execute.rd];
                }
                else
                {
                  cpu->decode.rs1_value = cpu->execute.result_buffer;
                }
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == memReg)
              {
                if(cpu->valid_regs[memReg] == 0)
                {
                  cpu->decode.rs1_value = cpu->regs[memReg];
                }
                else
                {
                  cpu->decode.rs1_value = memRes;
                }

                rs1_val = 1;
              }
              else
              {
                rs1_val = 0;
              }

              if(cpu->valid_regs[cpu->decode.rs2] == 0)
              {
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == cpu->execute.rd)
              {
                //printf("It is matching\n");
                if(cpu->valid_regs[cpu->execute.rd] == 0)
                {
                  //printf("1\n");
                  cpu->decode.rs2_value = cpu->regs[cpu->execute.rd];
                }
                else
                {
                  //printf("2\n");
                  cpu->decode.rs2_value = cpu->execute.result_buffer;
                }
                //printf("RS2 value = %d\n",cpu->decode.rs2_value);
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == memReg)
              {
                if(cpu->valid_regs[memReg] == 0)
                {
                  cpu->decode.rs2_value = cpu->regs[memReg];
                }
                else
                {
                  cpu->decode.rs2_value = memRes;
                }

                rs2_val = 1;
              }
              else
              {
                rs2_val = 0;
              }
              //printf("Register status in ADD %d-val=%d, %d-val=%d\n",rs1_val,cpu->decode.rs1_value,rs2_val,cpu->decode.rs2_value);
              if(rs1_val * rs2_val == 1)
              {
                cpu->valid_regs[cpu->decode.rd] = 1;
              }
              else
              {
                cpu->decode.stalled = 1;
                cpu->fetch.stalled = 1;
              }
                break;
            }

            case OPCODE_LOAD:
            {
              int rs1_val = 0;
              if(cpu->valid_regs[cpu->decode.rs1] == 0)
              {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == cpu->execute.rd)
              {
                cpu->decode.rs1_value = cpu->execute.result_buffer;
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == memReg)
              {
                cpu->decode.rs1_value = memRes;
                rs1_val = 1;
              }
              else
              {
                rs1_val = 0;
              }
              if(rs1_val == 1)
              {
                cpu->valid_regs[cpu->decode.rd] = 1;
              }
              else
              {
                cpu->decode.stalled = 1;
                cpu->fetch.stalled = 1;
              }
                break;
            }

            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                cpu->valid_regs[cpu->decode.rd] = 1;

                break;
            }

            case OPCODE_ADDL:
            {
              int rs1_val = 0;
              if(cpu->valid_regs[cpu->decode.rs1] == 0)
              {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == cpu->execute.rd)
              {
                cpu->decode.rs1_value = cpu->execute.result_buffer;
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == memReg)
              {
                cpu->decode.rs1_value = memRes;
                rs1_val = 1;
              }
              else
              {
                rs1_val = 0;
              }
              if(rs1_val == 1)
              {
                cpu->valid_regs[cpu->decode.rd] = 1;
              }
              else
              {
                cpu->decode.stalled = 1;
                cpu->fetch.stalled = 1;
              }
                break;
            }
            case OPCODE_SUB:
            {
              //printf("Inside decode subl\n");
              //printf("Execute, Memory rd is - R%d, R%d-----R%d\n",cpu->execute.rd,memReg,cpu->writeback.rd);

              int rs1_val , rs2_val = 0;
              if(cpu->valid_regs[cpu->decode.rs1] == 0)
              {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == cpu->execute.rd)
              {
                cpu->decode.rs1_value = cpu->execute.result_buffer;
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == memReg)
              {
                cpu->decode.rs1_value = memRes;
                rs1_val = 1;
              }
              else
              {
                rs1_val = 0;
              }

              if(cpu->valid_regs[cpu->decode.rs2] == 0)
              {
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == cpu->execute.rd)
              {
                cpu->decode.rs2_value = cpu->execute.result_buffer;
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == memReg)
              {
                cpu->decode.rs2_value = memRes;
                rs2_val = 1;
              }
              else
              {
                rs2_val = 0;
              }

              if(rs1_val * rs2_val == 1)
              {
                //printf("Values= %d,%d\n",cpu->decode.rs1_value,cpu->decode.rs2_value);
                cpu->valid_regs[cpu->decode.rd] = 1;
              }
              else
              {
                cpu->decode.stalled = 1;
                cpu->fetch.stalled = 1;
              }
                break;
            }
            case OPCODE_SUBL:
            {
              int rs1_val = 0;
              if(cpu->valid_regs[cpu->decode.rs1] == 0)
              {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == cpu->execute.rd)
              {
                cpu->decode.rs1_value = cpu->execute.result_buffer;
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == memReg)
              {
                cpu->decode.rs1_value = memRes;
                rs1_val = 1;
              }
              else
              {
                rs1_val = 0;
              }
              if(rs1_val == 1)
              {
                cpu->valid_regs[cpu->decode.rd] = 1;
              }
              else
              {
                cpu->decode.stalled = 1;
                cpu->fetch.stalled = 1;
              }
                break;
            }
            case OPCODE_MUL:
            {
                //printf("Register status: %d, %d\n", cpu->valid_regs[cpu->decode.rs1], cpu->valid_regs[cpu->decode.rs2]);
                int rs1_val , rs2_val = 0;
                if(cpu->valid_regs[cpu->decode.rs1] == 0)
                {
                  cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                  rs1_val = 1;
                }
                else if(cpu->decode.rs1 == cpu->execute.rd)
                {
                  if(cpu->valid_regs[cpu->execute.rd] == 0)
                  {
                    cpu->decode.rs1_value = cpu->regs[cpu->execute.rd];
                  }
                  else
                  {
                    cpu->decode.rs1_value = cpu->execute.result_buffer;
                  }
                  rs1_val = 1;
                }
                else if(cpu->decode.rs1 == memReg)
                {
                  if(cpu->valid_regs[memReg] == 0)
                  {
                    cpu->decode.rs1_value = cpu->regs[memReg];
                  }
                  else
                  {
                    cpu->decode.rs1_value = memRes;
                  }

                  rs1_val = 1;
                }
                else
                {
                  rs1_val = 0;
                }

                if(cpu->valid_regs[cpu->decode.rs2] == 0)
                {
                  cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                  rs2_val = 1;
                }
                else if(cpu->decode.rs2 == cpu->execute.rd)
                {
                  cpu->decode.rs2_value = cpu->execute.result_buffer;
                  rs2_val = 1;
                }
                else if(cpu->decode.rs2 == memReg)
                {
                  cpu->decode.rs2_value = memRes;
                  rs2_val = 1;
                }
                else
                {
                  rs2_val = 0;
                }

                if(rs1_val * rs2_val == 1)
                {
                  cpu->valid_regs[cpu->decode.rd] = 1;
                }
                else
                {
                  cpu->decode.stalled = 1;
                  cpu->fetch.stalled = 1;
                }
                  break;
            }
            case OPCODE_DIV:
            {
              int rs1_val , rs2_val = 0;
              if(cpu->valid_regs[cpu->decode.rs1] == 0)
              {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == cpu->execute.rd)
              {
                cpu->decode.rs1_value = cpu->execute.result_buffer;
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == memReg)
              {
                cpu->decode.rs1_value = memRes;
                rs1_val = 1;
              }
              else
              {
                rs1_val = 0;
              }

              if(cpu->valid_regs[cpu->decode.rs2] == 0)
              {
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == cpu->execute.rd)
              {
                cpu->decode.rs2_value = cpu->execute.result_buffer;
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == memReg)
              {
                cpu->decode.rs2_value = memRes;
                rs2_val = 1;
              }
              else
              {
                rs2_val = 0;
              }
              //printf("Division regs = %d,%d\n",cpu->decode.rs1_value,cpu->decode.rs2_value);
              if(rs1_val * rs2_val == 1)
              {
                cpu->valid_regs[cpu->decode.rd] = 1;
              }
              else
              {
                cpu->decode.stalled = 1;
                cpu->fetch.stalled = 1;
              }
                break;
            }
            case OPCODE_AND:
            {
              int rs1_val , rs2_val = 0;
              if(cpu->valid_regs[cpu->decode.rs1] == 0)
              {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == cpu->execute.rd)
              {
                cpu->decode.rs1_value = cpu->execute.result_buffer;
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == memReg)
              {
                cpu->decode.rs1_value = memRes;
                rs1_val = 1;
              }
              else
              {
                rs1_val = 0;
              }

              if(cpu->valid_regs[cpu->decode.rs2] == 0)
              {
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == cpu->execute.rd)
              {
                cpu->decode.rs2_value = cpu->execute.result_buffer;
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == memReg)
              {
                cpu->decode.rs2_value = memRes;
                rs2_val = 1;
              }
              else
              {
                rs2_val = 0;
              }

              if(rs1_val * rs2_val == 1)
              {
                cpu->valid_regs[cpu->decode.rd] = 1;
              }
              else
              {
                cpu->decode.stalled = 1;
                cpu->fetch.stalled = 1;
              }
                break;
            }
            case OPCODE_OR:
            {
              int rs1_val , rs2_val = 0;
              if(cpu->valid_regs[cpu->decode.rs1] == 0)
              {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == cpu->execute.rd)
              {
                cpu->decode.rs1_value = cpu->execute.result_buffer;
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == memReg)
              {
                cpu->decode.rs1_value = memRes;
                rs1_val = 1;
              }
              else
              {
                rs1_val = 0;
              }

              if(cpu->valid_regs[cpu->decode.rs2] == 0)
              {
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == cpu->execute.rd)
              {
                cpu->decode.rs2_value = cpu->execute.result_buffer;
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == memReg)
              {
                cpu->decode.rs2_value = memRes;
                rs2_val = 1;
              }
              else
              {
                rs2_val = 0;
              }

              if(rs1_val * rs2_val == 1)
              {
                cpu->valid_regs[cpu->decode.rd] = 1;
              }
              else
              {
                cpu->decode.stalled = 1;
                cpu->fetch.stalled = 1;
              }
                break;
            }
            case OPCODE_XOR:
            {
              int rs1_val , rs2_val = 0;
              if(cpu->valid_regs[cpu->decode.rs1] == 0)
              {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == cpu->execute.rd)
              {
                cpu->decode.rs1_value = cpu->execute.result_buffer;
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == memReg)
              {
                cpu->decode.rs1_value = memRes;
                rs1_val = 1;
              }
              else
              {
                rs1_val = 0;
              }

              if(cpu->valid_regs[cpu->decode.rs2] == 0)
              {
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == cpu->execute.rd)
              {
                cpu->decode.rs2_value = cpu->execute.result_buffer;
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == memReg)
              {
                cpu->decode.rs2_value = memRes;
                rs2_val = 1;
              }
              else
              {
                rs2_val = 0;
              }

              if(rs1_val * rs2_val == 1)
              {
                cpu->valid_regs[cpu->decode.rd] = 1;
              }
              else
              {
                cpu->decode.stalled = 1;
                cpu->fetch.stalled = 1;
              }
                break;
            }

            case OPCODE_LDR:
            {
              int rs1_val , rs2_val = 0;
              if(cpu->valid_regs[cpu->decode.rs1] == 0)
              {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == cpu->execute.rd)
              {
                cpu->decode.rs1_value = cpu->execute.result_buffer;
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == memReg)
              {
                cpu->decode.rs1_value = memRes;
                rs1_val = 1;
              }
              else
              {
                rs1_val = 0;
              }

              if(cpu->valid_regs[cpu->decode.rs2] == 0)
              {
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == cpu->execute.rd)
              {
                cpu->decode.rs2_value = cpu->execute.result_buffer;
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == memReg)
              {
                cpu->decode.rs2_value = memRes;
                rs2_val = 1;
              }
              else
              {
                rs2_val = 0;
              }

              if(rs1_val * rs2_val == 1)
              {
                cpu->valid_regs[cpu->decode.rd] = 1;
              }
              else
              {
                cpu->decode.stalled = 1;
                cpu->fetch.stalled = 1;
              }
                break;
            }

            case OPCODE_STORE:
            {
              int rs1_val , rs2_val = 0;
              if(cpu->valid_regs[cpu->decode.rs1] == 0)
              {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == cpu->execute.rd)
              {
                cpu->decode.rs1_value = cpu->execute.result_buffer;
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == memReg)
              {
                cpu->decode.rs1_value = memRes;
                rs1_val = 1;
              }
              else
              {
                rs1_val = 0;
              }

              if(cpu->valid_regs[cpu->decode.rs2] == 0)
              {
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == cpu->execute.rd)
              {
                cpu->decode.rs2_value = cpu->execute.result_buffer;
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == memReg)
              {
                cpu->decode.rs2_value = memRes;
                rs2_val = 1;
              }
              else
              {
                rs2_val = 0;
              }

              if(rs1_val * rs2_val == 1)
              {
                cpu->decode.rd = -1;
                //cpu->valid_regs[cpu->decode.rd] = 1;
              }
              else
              {
                cpu->decode.stalled = 1;
                cpu->fetch.stalled = 1;
              }
                break;
            }

            case OPCODE_STR:
            {
              int rs1_val , rs2_val, rs3_val  = 0;
              if(cpu->valid_regs[cpu->decode.rs1] == 0)
              {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == cpu->execute.rd)
              {
                cpu->decode.rs1_value = cpu->execute.result_buffer;
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == memReg)
              {
                cpu->decode.rs1_value = memRes;
                rs1_val = 1;
              }
              else
              {
                rs1_val = 0;
              }

              if(cpu->valid_regs[cpu->decode.rs2] == 0)
              {
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == cpu->execute.rd)
              {
                cpu->decode.rs2_value = cpu->execute.result_buffer;
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == memReg)
              {
                cpu->decode.rs2_value = memRes;
                rs2_val = 1;
              }
              else
              {
                rs2_val = 0;
              }

              if(cpu->valid_regs[cpu->decode.rs3] == 0)
              {
                cpu->decode.rs3_value = cpu->regs[cpu->decode.rs3];
                rs3_val = 1;
              }
              else if(cpu->decode.rs3 == cpu->execute.rd)
              {
                cpu->decode.rs3_value = cpu->execute.result_buffer;
                rs3_val = 1;
              }
              else if(cpu->decode.rs3 == memReg)
              {
                cpu->decode.rs3_value = memRes;
                rs3_val = 1;
              }
              else
              {
                rs3_val = 0;
              }

              if(rs1_val * rs2_val * rs3_val == 1)
              {
                //cpu->valid_regs[cpu->decode.rd] = 1;
                cpu->decode.rd = -1;
              }
              else
              {
                cpu->decode.stalled = 1;
                cpu->fetch.stalled = 1;
              }
                break;
            }

            case OPCODE_BZ:
            {
                /* BZ doesn't have register operands */
                cpu->decode.rd = -1;
                break;
            }

            case OPCODE_BNZ:
            {
                /* BNZ doesn't have register operands */
                cpu->decode.rd = -1;
                break;
            }

            case OPCODE_HALT:
            {
                /* HALT doesn't have register operands */
                cpu->decode.rd = -1;
                break;
            }

            case OPCODE_NOP:
            {
                /* NOP doesn't have register operands */
                cpu->decode.rd = -1;
                break;
            }

            case OPCODE_CMP:
            {
              int rs1_val , rs2_val = 0;
              if(cpu->valid_regs[cpu->decode.rs1] == 0)
              {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == cpu->execute.rd)
              {
                cpu->decode.rs1_value = cpu->execute.result_buffer;
                rs1_val = 1;
              }
              else if(cpu->decode.rs1 == memReg)
              {
                cpu->decode.rs1_value = memRes;
                rs1_val = 1;
              }
              else
              {
                rs1_val = 0;
              }

              if(cpu->valid_regs[cpu->decode.rs2] == 0)
              {
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == cpu->execute.rd)
              {
                cpu->decode.rs2_value = cpu->execute.result_buffer;
                rs2_val = 1;
              }
              else if(cpu->decode.rs2 == memReg)
              {
                cpu->decode.rs2_value = memRes;
                rs2_val = 1;
              }
              else
              {
                rs2_val = 0;
              }

              if(rs1_val * rs2_val == 1)
              {
                //printf("Values= %d,%d\n",cpu->decode.rs1_value,cpu->decode.rs2_value);
                //cpu->valid_regs[cpu->decode.rd] = 1;
                cpu->decode.rd = -1;
              }
              else
              {
                cpu->decode.stalled = 1;
                cpu->fetch.stalled = 1;
              }
                break;
            }

            default:
            {
                break;
            }
        }

        if(cpu->execute.stalled == 0 && cpu->decode.stalled == 0){
            /* Copy data from decode latch to execute latch*/
            //printf("Invalidated Register= R%d\n",cpu->decode.rd);
            //cpu->valid_regs[cpu->decode.rd] = 1;
            cpu->execute = cpu->decode;
            exeEmpty = 0;

            if (ENABLE_DEBUG_MESSAGES)
            {
                print_stage_content("Decode/RF", &cpu->decode);
            }
            //cpu->decode.has_insn = FALSE;

        }
        else
        {
            if (ENABLE_DEBUG_MESSAGES)
            {
              if(cpu->decode.has_insn == FALSE)
              {
                printf("Decode/RF      :     EMPTY\n");
              }
              else
              {
                print_stage_content("Decode/RF", &cpu->decode);
              }
                exeEmpty = 1;
                //cpu->decode.has_insn = FALSE;
                //printf("val --%d, %d\n",cpu->execute.stalled, cpu->decode.stalled );
            }
            //cpu->decode.stalled = 1;
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES)
            {
              if(cpu->decode.has_insn == FALSE)
              {
                printf("Decode/RF      :     EMPTY\n");
              }
              else
              {
                print_stage_content("Decode/RF", &cpu->decode);
              }
                exeEmpty = 1;
                //cpu->decode.has_insn = FALSE;
                //printf("val --%d, %d\n",cpu->execute.stalled, cpu->decode.stalled );
            }
    }

}

/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_execute(APEX_CPU *cpu)
{
    if (cpu->execute.has_insn && cpu->execute.stalled == 0)
    {
      exeEmpty = 0;
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.rs2_value;
                //printf("Values being added in execute = %d, %d\n",cpu->execute.rs1_value , cpu->execute.rs2_value);
                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                }
                else
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;
                break;
            }

            case OPCODE_BZ:
            {

                if (cpu->zero_flag == TRUE)
                {
                  if(strncmp(cpu->decode.opcode_str, "HALT",1) == 0)
                  {
                    break;
                  }
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;

                    /* Since we are using reverse callbacks for pipeline stages,
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;

                    cpu->decode.stalled = 0;
                    cpu->fetch.stalled = 0;
                }
                break;
            }

            case OPCODE_BNZ:
            {
                if (cpu->zero_flag == FALSE)
                {
                  if(strncmp(cpu->decode.opcode_str, "HALT",1) == 0)
                  {
                    break;
                  }
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;

                    /* Since we are using reverse callbacks for pipeline stages,
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_MOVC:
            {
                cpu->execute.result_buffer = cpu->execute.imm;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                }
                else
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_ADDL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.imm;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                }
                else
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_SUB:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value - cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                }
                else
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_SUBL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value - cpu->execute.imm;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                }
                else
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_MUL:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value * cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                }
                else
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_DIV:
            {
                if (cpu->execute.rs2_value != 0)
                {
                  cpu->execute.result_buffer = cpu->execute.rs1_value / cpu->execute.rs2_value;
                }
                else
                {
                  fprintf(stderr, "Division By Zero Returning Value Zero\n");
                  cpu->execute.result_buffer = 0;
                }

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                }
                else
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_AND:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value & cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                }
                else
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_OR:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value | cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                }
                else
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_XOR:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value ^ cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                }
                else
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            case OPCODE_LDR:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.rs2_value;
                break;
            }

            case OPCODE_STORE:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value;

                cpu->execute.memory_address
                    = cpu->execute.rs2_value + cpu->execute.imm;
                break;
            }

            case OPCODE_STR:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value;

                cpu->execute.memory_address
                    = cpu->execute.rs2_value + cpu->execute.rs3_value;

                break;
            }

            case OPCODE_HALT:
            {
                /* HALT doesn't have register operands */
                break;
            }

            case OPCODE_NOP:
            {
                /* NOP doesn't have register operands */
                break;
            }

            case OPCODE_CMP:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value - cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                }
                else
                {
                    cpu->zero_flag = FALSE;
                }
                break;
            }

            default:
            {
                break;
            }
        }

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;
        memEmpty = 0;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Execute", &cpu->execute);
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("Execute         :   EMPTY\n");
            memEmpty = 1;
        }
    }

}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu)
{
    if (cpu->memory.has_insn)
    {
      memEmpty = 0;
      memReg = cpu->memory.rd;
      memRes = cpu->memory.result_buffer;
        switch (cpu->memory.opcode)
        {

            case OPCODE_ADD:
            {
                /* No work for ADD */
                break;
            }

            case OPCODE_LOAD:
            {
                /* Read from data memory */
                cpu->memory.result_buffer
                    = cpu->data_memory[cpu->memory.memory_address];
                break;
            }

            case OPCODE_LDR:
            {
                /* Read from data memory */
                cpu->memory.result_buffer
                    = cpu->data_memory[cpu->memory.memory_address];
                break;
            }

            case OPCODE_STORE:
            {
                /* Read from data memory */
                    cpu->data_memory[cpu->memory.memory_address] = cpu->memory.result_buffer;
                break;
            }

            case OPCODE_STR:
            {
                /* Read from data memory */
                cpu->data_memory[cpu->memory.memory_address] = cpu->memory.result_buffer;
                break;
            }

            case OPCODE_MOVC:
            {
                /* Read from data memory */
                //printf("Destination res=R%d\n",cpu->memory.rd);
                break;
            }

            default:
            {
                break;
            }
        }

        /* Copy data from memory latch to writeback latch*/
        cpu->writeback = cpu->memory;
        cpu->memory.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory", &cpu->memory);
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("Memory          :  Empty\n");
            memEmpty = 1;
        }
    }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{

    if (cpu->writeback.has_insn)
    {
      int val = 0;
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                //printf("Result value ADD = %d\n",cpu->writeback.result_buffer);
                //printf("Execute Regs, Memory Regs = R%d,R%d\n",cpu->execute.rd,cpu->memory.rd);

                if(memEmpty == 0 && exeEmpty == 0 && (cpu->memory.rd == cpu->writeback.rd || cpu->execute.rd == cpu->writeback.rd))
                {
                  val = 1;
                }
                else if(memEmpty == 1 && exeEmpty == 0 && cpu->execute.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                else if(memEmpty == 0 && exeEmpty == 1 && cpu->memory.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                if(val == 0)
                {
                  cpu->valid_regs[cpu->writeback.rd] = 0;
                  cpu->fetch.stalled = 0;
                  cpu->decode.stalled = 0;
                }
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;

                if(memEmpty == 0 && exeEmpty == 0 && (cpu->memory.rd == cpu->writeback.rd || cpu->execute.rd == cpu->writeback.rd))
                {
                  val = 1;
                }
                else if(memEmpty == 1 && exeEmpty == 0 && cpu->execute.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                else if(memEmpty == 0 && exeEmpty == 1 && cpu->memory.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                if(val == 0)
                {
                  cpu->valid_regs[cpu->writeback.rd] = 0;
                  cpu->fetch.stalled = 0;
                  cpu->decode.stalled = 0;
                }
                break;
            }

            case OPCODE_MOVC:
            {
                  cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                  //printf("Execute Regs, Memory Regs, wb REGS = R%d,R%d,R%d\n",cpu->execute.rd,cpu->memory.rd,cpu->writeback.rd);
                  //printf("Execute instruction = %s\n",cpu->execute.opcode_str);

                  if(memEmpty == 0 && exeEmpty == 0 && (cpu->memory.rd == cpu->writeback.rd || cpu->execute.rd == cpu->writeback.rd))
                  {
                    printf("4\n");
                    val = 1;
                  }
                  else if(memEmpty == 1 && exeEmpty == 0 && cpu->execute.rd == cpu->writeback.rd)
                  {
                    val = 1;
                  }
                  else if(memEmpty == 0 && exeEmpty == 1 && cpu->memory.rd == cpu->writeback.rd)
                  {
                    val = 1;
                  }
                  if(val == 0)
                  {
                    cpu->valid_regs[cpu->writeback.rd] = 0;
                    cpu->fetch.stalled = 0;
                    cpu->decode.stalled = 0;
                  }
                  break;
            }

            case OPCODE_ADDL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;

                if(memEmpty == 0 && exeEmpty == 0 && (cpu->memory.rd == cpu->writeback.rd || cpu->execute.rd == cpu->writeback.rd))
                {
                  val = 1;
                }
                else if(memEmpty == 1 && exeEmpty == 0 && cpu->execute.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                else if(memEmpty == 0 && exeEmpty == 1 && cpu->memory.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                if(val == 0)
                {
                  cpu->valid_regs[cpu->writeback.rd] = 0;
                  cpu->fetch.stalled = 0;
                  cpu->decode.stalled = 0;
                }
                break;
            }
            case OPCODE_SUB:
            {
              cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;

              if(memEmpty == 0 && exeEmpty == 0 && (cpu->memory.rd == cpu->writeback.rd || cpu->execute.rd == cpu->writeback.rd))
              {
                val = 1;
              }
              else if(memEmpty == 1 && exeEmpty == 0 && cpu->execute.rd == cpu->writeback.rd)
              {
                val = 1;
              }
              else if(memEmpty == 0 && exeEmpty == 1 && cpu->memory.rd == cpu->writeback.rd)
              {
                val = 1;
              }
              if(val == 0)
              {
                cpu->valid_regs[cpu->writeback.rd] = 0;
                cpu->fetch.stalled = 0;
                cpu->decode.stalled = 0;
              }
                break;
            }
            case OPCODE_SUBL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;

                if(memEmpty == 0 && exeEmpty == 0 && (cpu->memory.rd == cpu->writeback.rd || cpu->execute.rd == cpu->writeback.rd))
                {
                  val = 1;
                }
                else if(memEmpty == 1 && exeEmpty == 0 && cpu->execute.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                else if(memEmpty == 0 && exeEmpty == 1 && cpu->memory.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                if(val == 0)
                {
                  cpu->valid_regs[cpu->writeback.rd] = 0;
                  cpu->fetch.stalled = 0;
                  cpu->decode.stalled = 0;
                }
                break;
            }
            case OPCODE_MUL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;

                if(memEmpty == 0 && exeEmpty == 0 && (cpu->memory.rd == cpu->writeback.rd || cpu->execute.rd == cpu->writeback.rd))
                {
                  val = 1;
                }
                else if(memEmpty == 1 && exeEmpty == 0 && cpu->execute.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                else if(memEmpty == 0 && exeEmpty == 1 && cpu->memory.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                if(val == 0)
                {
                  cpu->valid_regs[cpu->writeback.rd] = 0;
                  cpu->fetch.stalled = 0;
                  cpu->decode.stalled = 0;
                }
                break;
            }
            case OPCODE_DIV:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;

                if(memEmpty == 0 && exeEmpty == 0 && (cpu->memory.rd == cpu->writeback.rd || cpu->execute.rd == cpu->writeback.rd))
                {
                  val = 1;
                }
                else if(memEmpty == 1 && exeEmpty == 0 && cpu->execute.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                else if(memEmpty == 0 && exeEmpty == 1 && cpu->memory.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                if(val == 0)
                {
                  cpu->valid_regs[cpu->writeback.rd] = 0;
                  cpu->fetch.stalled = 0;
                  cpu->decode.stalled = 0;
                }
                break;
            }
            case OPCODE_AND:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;

                if(memEmpty == 0 && exeEmpty == 0 && (cpu->memory.rd == cpu->writeback.rd || cpu->execute.rd == cpu->writeback.rd))
                {
                  val = 1;
                }
                else if(memEmpty == 1 && exeEmpty == 0 && cpu->execute.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                else if(memEmpty == 0 && exeEmpty == 1 && cpu->memory.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                if(val == 0)
                {
                  cpu->valid_regs[cpu->writeback.rd] = 0;
                  cpu->fetch.stalled = 0;
                  cpu->decode.stalled = 0;
                }
                break;
            }
            case OPCODE_OR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;

                if(memEmpty == 0 && exeEmpty == 0 && (cpu->memory.rd == cpu->writeback.rd || cpu->execute.rd == cpu->writeback.rd))
                {
                  val = 1;
                }
                else if(memEmpty == 1 && exeEmpty == 0 && cpu->execute.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                else if(memEmpty == 0 && exeEmpty == 1 && cpu->memory.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                if(val == 0)
                {
                  cpu->valid_regs[cpu->writeback.rd] = 0;
                  cpu->fetch.stalled = 0;
                  cpu->decode.stalled = 0;
                }
                break;
            }
            case OPCODE_XOR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;

                if(memEmpty == 0 && exeEmpty == 0 && (cpu->memory.rd == cpu->writeback.rd || cpu->execute.rd == cpu->writeback.rd))
                {
                  val = 1;
                }
                else if(memEmpty == 1 && exeEmpty == 0 && cpu->execute.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                else if(memEmpty == 0 && exeEmpty == 1 && cpu->memory.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                if(val == 0)
                {
                  cpu->valid_regs[cpu->writeback.rd] = 0;
                  cpu->fetch.stalled = 0;
                  cpu->decode.stalled = 0;
                }
                break;
            }

            case OPCODE_LDR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;

                if(memEmpty == 0 && exeEmpty == 0 && (cpu->memory.rd == cpu->writeback.rd || cpu->execute.rd == cpu->writeback.rd))
                {
                  val = 1;
                }
                else if(memEmpty == 1 && exeEmpty == 0 && cpu->execute.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                else if(memEmpty == 0 && exeEmpty == 1 && cpu->memory.rd == cpu->writeback.rd)
                {
                  val = 1;
                }
                if(val == 0)
                {
                  cpu->valid_regs[cpu->writeback.rd] = 0;
                  cpu->fetch.stalled = 0;
                  cpu->decode.stalled = 0;
                }
                break;
            }

            case OPCODE_STORE:
            {
                break;
            }

            case OPCODE_STR:
            {
                break;
            }

            case OPCODE_BZ:
            {
                /* BZ doesn't have register operands */

                break;
            }

            case OPCODE_BNZ:
            {
                /* BNZ doesn't have register operands */

                break;
            }

            case OPCODE_HALT:
            {
                /* HALT doesn't have register operands */
                break;
            }

            case OPCODE_NOP:
            {
                /* NOP doesn't have register operands */

                break;
            }

            case OPCODE_CMP:
            {
                break;
            }

            default:
            {
                break;
            }
        }

        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Writeback", &cpu->writeback);
        }

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("Writeback      :  Empty\n");
        }
    }
    /* Default */
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    cpu->single_step = ENABLE_SINGLE_STEP;
    cpu->fetch.stalled = 0;
    cpu->decode.stalled = 0;
    cpu->execute.stalled = 0;
    cpu->memory.stalled = 0;
    cpu->writeback.stalled = 0;
    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}

int print_register_state(APEX_CPU* cpu) {
  printf("\n=============== STATE OF ARCHITECTURAL REGISTER FILE ==========\n");
  int index;
  int no_registers = (int) (sizeof(cpu->regs)/sizeof(cpu->regs[0]));
  for(index = 0; index < no_registers - 1; ++index) {//Assumming CC register is also part of the register file
    printf("| \t REG[%d] \t | \t Value = %d \t | \t Status = %s \t \n", index, cpu->regs[index], (!cpu->valid_regs[index] ? "VALID" : "INVALID"));
  }
  return 0;
}

int print_data_memory(APEX_CPU* cpu) {
  printf("\n============== STATE OF DATA MEMORY =============\n");
  int index;
  for(index = 0; index < 1000; ++index) {
    printf("| \t MEM[%d] \t | \t Data Value = %d \t |\n", index, cpu->data_memory[index]);
  }
  return 0;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;

    while (TRUE)
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock+1);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        print_reg_file(cpu);

        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
                break;
            }
        }

        cpu->clock++;
    }

    print_register_state(cpu);
    print_data_memory(cpu);
}


/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_simulate(APEX_CPU *cpu, int c)
{
    char user_prompt_val;

    while (c != 1)
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock+1);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        print_reg_file(cpu);



        cpu->clock++;
        c--;
    }
    while(TRUE)
    {
      if (ENABLE_DEBUG_MESSAGES)
      {
          printf("--------------------------------------------\n");
          printf("Clock Cycle #: %d\n", cpu->clock+1);
          printf("--------------------------------------------\n");
      }

      if (APEX_writeback(cpu))
      {
          /* Halt in writeback stage */
          printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
          break;
      }

      APEX_memory(cpu);
      APEX_execute(cpu);
      APEX_decode(cpu);
      APEX_fetch(cpu);

      print_reg_file(cpu);
      if (cpu->single_step)
      {
          printf("Press any key to advance CPU Clock or <q> to quit:\n");
          scanf("%c", &user_prompt_val);

          if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
          {
              printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
              break;
          }
          cpu->clock++;
      }
    }

    print_register_state(cpu);
    print_data_memory(cpu);
}


/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_display(APEX_CPU *cpu)
{
    int c = 10;
    while (c != 0)
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock+1);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        print_reg_file(cpu);

        cpu->clock++;
        c--;
    }

    print_register_state(cpu);
    print_data_memory(cpu);
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}
