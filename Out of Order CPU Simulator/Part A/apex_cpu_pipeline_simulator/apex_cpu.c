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
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
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

        case OPCODE_ADDL:
        case OPCODE_SUBL:
        case OPCODE_LDI:
        case OPCODE_LOAD:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_STI:
        case OPCODE_STORE:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }

        case OPCODE_BP:
        case OPCODE_BNP:
        case OPCODE_BZ:
        case OPCODE_BNZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_NOP:
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

        case OPCODE_JUMP:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rs1, stage->imm);
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
    printf("%-15s (I%d: %d) ", name, (stage->pc-4000)/4, stage->pc);
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
    printf("\n--%s--\n", "STATE OF ARCHITECTURAL REGISTER FILE");

    for (int i = 0; i < REG_FILE_SIZE; ++i)
    {
        printf("| REG[%-2d] | Value=%-4d | Status=%-7s |\n", i, cpu->regs[i],(cpu->regf[i]==0)?"VALID":"INVALID");
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
            //printf("Instruction at FETCH_____STAGE ---> EMPTY\n");
            cpu->fp=0;

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
        cpu->fetch.imm = current_ins->imm;

        /* Update PC for next instruction */
        cpu->pc += 4;

        /* Copy data from fetch latch to decode latch*/
        cpu->decode = cpu->fetch;
        cpu->pfetch = cpu->fetch;

        if (ENABLE_DEBUG_MESSAGES)
        {
            //print_stage_content("Instruction at FETCH_____STAGE --->", &cpu->fetch);
        }

        /* Stop fetching new instructions if HALT is fetched */
        if (cpu->fetch.opcode == OPCODE_HALT)
        {
            cpu->fetch.has_insn = FALSE;
            //cpu->fp=0;
        }
    }
    else{
    cpu->fp=0;
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
    if (cpu->decode.has_insn)
    {
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_STORE:
            case OPCODE_STI:
            case OPCODE_CMP:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                break;
            }

            case OPCODE_LOAD:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_LDI:
            case OPCODE_JUMP:
            {
                cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                break;
            }

            case OPCODE_MOVC:
            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_HALT:
            case OPCODE_NOP:
            {
                /* doesn't have register operands */
                break;
            }

            
            
        }

        /* Copy data from decode latch to execute latch*/
        cpu->execute = cpu->decode;
        cpu->pdecode = cpu->decode;
        cpu->decode.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            //print_stage_content("Instruction at DECODE_RF_STAGE --->", &cpu->decode);
        }
    }
    else{
            cpu->dp=0;
            //printf("Instruction at DECODE_RF_STAGE ---> EMPTY\n");
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
    
    if (cpu->execute.has_insn)
    {
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {
            case OPCODE_ADD:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value + cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
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

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
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

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
                }

                break;
            }

            case OPCODE_DIV:
            {
                cpu->execute.result_buffer
                    = cpu->execute.rs1_value / cpu->execute.rs2_value;

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
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

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
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

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
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

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
                }

                break;
            }

            case OPCODE_LOAD:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;
                break;
            }

            case OPCODE_LDI:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;

                cpu->execute.result_buffer1 = cpu->execute.rs1_value + 4;
                break;
            }

            case OPCODE_BZ:
            {
                if (cpu->zero_flag == TRUE)
                {
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

            case OPCODE_BNZ:
            {
                if (cpu->zero_flag == FALSE)
                {
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

            case OPCODE_BP:
            {
                if (cpu->positive_flag == TRUE)
                {
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

            case OPCODE_BNP:
            {
                if (cpu->positive_flag == FALSE)
                {
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
                cpu->execute.result_buffer = cpu->execute.imm+0;

                

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

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
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

                if (cpu->execute.result_buffer > 0)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
                }

                break;
            }

            case OPCODE_STORE:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs2_value + cpu->execute.imm;
                break;
            }

            case OPCODE_STI:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs2_value + cpu->execute.imm;

                cpu->execute.result_buffer = cpu->execute.rs2_value + 4;
                break;
            }

            case OPCODE_CMP:
            {
                

                /* Set the zero flag based on the result buffer */
                if (cpu->execute.rs1_value == cpu->execute.rs2_value)
                {
                    cpu->zero_flag = TRUE;
                } 
                else 
                {
                    cpu->zero_flag = FALSE;
                }

                if (cpu->execute.rs1_value > cpu->execute.rs2_value)
                {
                    cpu->positive_flag = TRUE;
                } 
                else 
                {
                    cpu->positive_flag = FALSE;
                }

                break;
            }

            case OPCODE_JUMP:
            {
                cpu->pc= cpu->execute.rs1_value + cpu->execute.imm;

                /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;

                break;
            }

            case OPCODE_HALT:
            case OPCODE_NOP:
            {
                /* No work for these instructions */
                break;
            }

        }

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->pexecute = cpu->execute;
        //cpu->execute.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            //print_stage_content("Instruction at EX________STAGE --->", &cpu->execute);
        }
    }
    else{
            cpu->ep=0;
            //printf("Instruction at EX________STAGE ---> EMPTY\n");
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
    if(cpu->stall_count==2){
        cpu->stall_count=1;
    }
    else if(cpu->stall_count==1){
        cpu->stall_count=0;
    }
    if (cpu->memory.has_insn)
    {
        switch (cpu->memory.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_MOVC:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_CMP:
            case OPCODE_JUMP:
            case OPCODE_HALT:
            case OPCODE_NOP:
            {
                /* No work for these instructions */
                break;
            }

            case OPCODE_LOAD:
            case OPCODE_LDI:
            {
                /* Read from data memory */
                cpu->memory.result_buffer
                    = cpu->data_memory[cpu->memory.memory_address];
                break;
            }

            case OPCODE_STORE:
            case OPCODE_STI:
            {
                /* Read from data memory */
                cpu->data_memory[cpu->memory.memory_address]
                    = cpu->memory.rs1_value;
                break;
            }

        }

        /* Copy data from memory latch to writeback latch*/
        cpu->writeback = cpu->memory;
        cpu->pmemory = cpu->memory;
        //cpu->memory.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            //print_stage_content("Instruction at MEMORY____STAGE --->", &cpu->memory);
        }
    }

    else{
            cpu->mp=0;
            //printf("Instruction at MEMORY____STAGE ---> EMPTY\n");
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
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_MOVC:
            case OPCODE_LOAD:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }

            case OPCODE_LDI:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->regs[cpu->writeback.rs1] = cpu->writeback.result_buffer1;
                break;
            }

            case OPCODE_STI: 
            {
                cpu->regs[cpu->writeback.rs2] = cpu->writeback.result_buffer;
                break;
            }

            case OPCODE_STORE: 
            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_CMP:
            case OPCODE_JUMP:
            case OPCODE_HALT:
            case OPCODE_NOP:
            {
                /* No work for these instructions */
                break;
            }

        }

        cpu->insn_completed++;
        cpu->pwriteback = cpu->writeback;
        cpu->writeback.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            //print_stage_content("Instruction at WRITEBACK_STAGE --->", &cpu->writeback);
        }

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }
    }
    else{
            cpu->wp=0;
            //printf("Instruction at WRITEBACK_STAGE ---> EMPTY\n");
    }

    /* Default */
    return 0;
}

static void
get_mem_dest(APEX_CPU *cpu)
{
    if (cpu->memory.has_insn)
    {
        switch (cpu->memory.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_MOVC:
            case OPCODE_LOAD:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            {
                cpu->mem_dest=cpu->memory.rd;
                cpu->mem_dest1=-1;
                break;
            }

            case OPCODE_LDI:
            {
                cpu->mem_dest=cpu->memory.rd;
                cpu->mem_dest1=cpu->memory.rs1;
                break;
            }

            case OPCODE_STI:
            {
                cpu->mem_dest=cpu->memory.rs2;
                cpu->mem_dest1=-1;
                break;
            }

        }

    }
    cpu->memory.has_insn = FALSE;

}


static void
get_ex_dest(APEX_CPU *cpu)
{
    if (cpu->execute.has_insn)
    {
        switch (cpu->execute.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_MOVC:
            case OPCODE_LOAD:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            {
                cpu->ex_dest=cpu->execute.rd;
                cpu->ex_dest1=-1;
                break;
            }

            case OPCODE_LDI:
            {
                cpu->ex_dest=cpu->execute.rd;
                cpu->ex_dest1=cpu->execute.rs1;
                break;
            }

            case OPCODE_STI:
            {
                cpu->ex_dest=cpu->execute.rs2;
                cpu->ex_dest1=-1;
                break;
            }

        }

    }
    cpu->execute.has_insn = FALSE;

}

static void
check_mem_dependency(APEX_CPU *cpu)
{
    if (cpu->decode.has_insn)
    {
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_STORE:
            case OPCODE_STI:
            case OPCODE_CMP:
            {
                if((cpu->decode.rs1==cpu->mem_dest) || (cpu->decode.rs2==cpu->mem_dest)){
                    cpu->stall_count=1;
                    cpu->regf[cpu->mem_dest]=1;
                    break;
                }
                if((cpu->decode.rs1==cpu->mem_dest1) || (cpu->decode.rs2==cpu->mem_dest1)){
                    cpu->stall_count=1;
                    cpu->regf[cpu->mem_dest1]=1;
                }
                break;
            }

            case OPCODE_LOAD:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_LDI:
            case OPCODE_JUMP:
            {
                
                if(cpu->decode.rs1==cpu->mem_dest){
                    cpu->stall_count=1;
                    cpu->regf[cpu->mem_dest]=1;
                }
                if((cpu->decode.rs1==cpu->mem_dest1)){
                    cpu->stall_count=1;
                    cpu->regf[cpu->mem_dest1]=1;
                }

                
                break;
            }

            case OPCODE_MOVC:
            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_HALT:
            case OPCODE_NOP:
            {
                /* doesn't have register operands */
                break;
            }

            
            
        }
        cpu->mem_dest=-1;
        cpu->mem_dest1=-1;
        /* Copy data from decode latch to execute latch*/
     }   
}


static void
check_ex_dependency(APEX_CPU *cpu)
{
    if (cpu->decode.has_insn)
    {
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_STORE:
            case OPCODE_STI:
            case OPCODE_CMP:
            {
                
                if((cpu->decode.rs1==cpu->ex_dest) || (cpu->decode.rs2==cpu->ex_dest)){
                    cpu->stall_count=2;
                    cpu->regf[cpu->ex_dest]=1;
                    break;
                }
                if((cpu->decode.rs1==cpu->ex_dest1) || (cpu->decode.rs2==cpu->ex_dest1)){
                    cpu->stall_count=2;
                    cpu->regf[cpu->ex_dest1]=1;
                }
                break;
            }

            case OPCODE_LOAD:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_LDI:
            case OPCODE_JUMP:
            {

                if(cpu->decode.rs1==cpu->ex_dest){
                    cpu->stall_count=2;
                    cpu->regf[cpu->ex_dest]=1;
                }
                if((cpu->decode.rs1==cpu->ex_dest1)){
                    cpu->stall_count=2;
                    cpu->regf[cpu->ex_dest1]=1;
                }
                break;
            }

            case OPCODE_MOVC:
            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_BP:
            case OPCODE_BNP:
            case OPCODE_HALT:
            case OPCODE_NOP:
            {
                /* doesn't have register operands */
                break;
            }

            
            
        }
        cpu->ex_dest=-1;
        cpu->ex_dest1=-1;
        /* Copy data from decode latch to execute latch*/
     }   
}

static void
stall_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->fetch.has_insn)
    {
        /* This fetches new branch target instruction from next cycle 
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;
            printf("Instruction at FETCH_____STAGE ---> EMPTY\n");

             Skip this cycle
            return;
        }*/

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
        cpu->fetch.imm = current_ins->imm;

        cpu->pdecode = cpu->decode;
        cpu->pfetch = cpu->fetch;

        /*
        cpu->pc += 4;

        cpu->decode = cpu->fetch;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Instruction at FETCH_____STAGE --->", &cpu->fetch);
        }

        
        if (cpu->fetch.opcode == OPCODE_HALT)
        {
            cpu->fetch.has_insn = FALSE;
        }
        */
    }
}


static void
print_data_mem(const APEX_CPU *cpu, int size)
{

    printf("--%s--\n", "STATE OF DATA MEMORY");

    for (int i = 0; i < size; ++i)
    {
        printf("| MEM[%-4d] | Data Value=%-4d |\n", i, cpu->data_memory[i]);
    }

    printf("\n");
}


/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    //int i;
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

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    /*if (ENABLE_DEBUG_MESSAGES)
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
            printf("%-10s %-10d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }*/

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    cpu->stall_count=0;
    return cpu;
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu,char func[],char cycle[])
{
    char user_prompt_val;
    cpu->clock=1;
    while (TRUE)
    {
        
        cpu->wp=1;
        cpu->mp=1;
        cpu->ep=1;
        cpu->dp=1;
        cpu->fp=1;

        for (int i = 0; i < REG_FILE_SIZE; ++i)
    {
        cpu->regf[i]=0;
    }

        /*if (ENABLE_DEBUG_MESSAGES)
        {
            printf("_ _ _ _ _ _ _ _ _ _ _ _CLOCK CYCLE %d_ _ _ _ _ _ _ _ _ _ _ _\n", cpu->clock);
        }*/

        int stop=APEX_writeback(cpu);
        
        APEX_memory(cpu);

        get_mem_dest(cpu);
        
        APEX_execute(cpu);
        
        get_ex_dest(cpu);

        check_mem_dependency(cpu);
        check_ex_dependency(cpu);
        
        if(cpu->stall_count==0){
            APEX_decode(cpu);
            APEX_fetch(cpu);
        }
        else{
            //print_stage_content("Instruction at DECODE_RF_STAGE --->", &cpu->decode);
            stall_fetch(cpu);
            //print_stage_content("Instruction at FETCH_____STAGE --->", &cpu->fetch);
        }


        if(strcmp(func, "display") == 0 || strcmp(func, "single_step") == 0){

            printf("\n_ _ _ _ _ _ _ _ _ _ _ _CLOCK CYCLE %d_ _ _ _ _ _ _ _ _ _ _ _\n", cpu->clock);

            if(cpu->fp==1){
                print_stage_content("\nInstruction at FETCH_____STAGE --->", &cpu->pfetch);
            }
            else{
                printf("Instruction at FETCH_____STAGE ---> EMPTY\n");
            }

            if(cpu->dp==1){
                print_stage_content("Instruction at DECODE_RF_STAGE --->", &cpu->pdecode);
            }
            else{
                printf("Instruction at DECODE_RF_STAGE ---> EMPTY\n");
            }
        
            if(cpu->ep==1){
                print_stage_content("Instruction at EX________STAGE --->", &cpu->pexecute);
            }
            else{
                printf("Instruction at EX________STAGE ---> EMPTY\n");
            }

            if(cpu->mp==1){
                print_stage_content("Instruction at MEMORY____STAGE --->", &cpu->pmemory);
            }
            else{
                printf("Instruction at MEMORY____STAGE ---> EMPTY\n");
            }
        
            if(cpu->wp==1){
                print_stage_content("Instruction at WRITEBACK_STAGE --->", &cpu->pwriteback);
            }
            else{
                printf("Instruction at WRITEBACK_STAGE ---> EMPTY\n");
            }

        }
        
        if(strcmp(func, "simulate") == 0 || strcmp(func, "display") == 0){
            if(cpu->clock==atoi(cycle)){
                stop=TRUE;
            }
        }

        //print_reg_file(cpu);

        if(stop==TRUE){

            if(strcmp(func, "show_mem") == 0){
                int memloc=atoi(cycle);
                printf("| MEM[%-4d] | Data Value=%-4d |\n", memloc, cpu->data_memory[memloc]);
                break;
            }

            if(strcmp(func, "simulate") == 0){
                print_reg_file(cpu);
                print_data_mem(cpu,DATA_MEMORY_SIZE);
                break;
            }

            if(strcmp(func, "display") == 0){
                printf("\n-----Flag Registers-----\nZero Flag:%d\nPositive Flag:%d\n",cpu->zero_flag,cpu->positive_flag);
                print_reg_file(cpu);
                print_data_mem(cpu,10);
                break;
            }
            
            /* Halt in writeback stage */
            //printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            break;
        }
        //print_data_mem(cpu);

        cpu->clock++;

        if (strcmp(func, "single_step") == 0)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                //printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                print_reg_file(cpu);
                print_data_mem(cpu,DATA_MEMORY_SIZE);
                break;
            }
        }

        
    }
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