module stack_structural_normal(
    inout wire[3:0] IO_DATA, 
    input wire RESET, 
    input wire CLK, 
    input wire[1:0] COMMAND,
    input wire[2:0] INDEX
    ); 
    
    stack _stack(IO_DATA, CLK, RESET, COMMAND, INDEX);

endmodule //stack_structural_normal

module RS_Trigger_Sync(
    output wire Q,
    output wire NQ,
    
    input wire R,
    input wire C,
    input wire S
);
    and(out_and_R_C, R, C);
    and(out_and_C_S, S, C);

    nor(Q, out_and_R_C, NQ);
    nor(NQ, out_and_C_S, Q);
endmodule //RS_Trigger_Sync

module D_Trigger(
    output wire Q,
    output wire NQ,
    
    input wire RESET,
    input wire D,
    input wire C
);  

    not(N_RESET, RESET);

    and(out_and_NRES_D, N_RESET, D);
    or(out_or_RES_C, C, RESET);

    not(not_out_and_NRES_D, out_and_NRES_D);

    RS_Trigger_Sync rs(
        .Q(Q),
        .NQ(NQ),

        .R(not_out_and_NRES_D),
        .C(out_or_RES_C),
        .S(out_and_NRES_D)
    );
endmodule //D_Trigger

module decode_3_bits_to_mod_5(
    output wire Q0,
    output wire Q1,
    output wire Q2,
    output wire Q3,
    output wire Q4,

    input wire A0,
    input wire A1,
    input wire A2
);

not(nA0, A0);
not(nA1, A1);
not(nA2, A2);

and(notA0_notA1_notA2, nA0, nA1, nA2);
and(A0_notA1_notA2, A0, nA1, nA2);
and(notA0_A1_notA2, nA0, A1, nA2);
and(A0_A1_notA2, A0, A1, nA2);
and(notA0_notA1_A2, nA0, nA1, A2);
and(A0_notA1_A2, A0, nA1, A2);
and(notA0_A1_A2, nA0, A1, A2);
and(A0_A1_A2, A0, A1, A2);

or(Q0, notA0_notA1_notA2, A0_notA1_A2);
or(Q1, A0_notA1_notA2, notA0_A1_A2);
or(Q2, notA0_A1_notA2, A0_A1_A2);


assign Q3 = A0_A1_notA2;
assign Q4 = notA0_notA1_A2;

endmodule //decode_3_bits_to_mod_5

module encode_mod_5_to_3_bits(
    output wire A0,
    output wire A1,
    output wire A2,

    input wire Q0,
    input wire Q1,
    input wire Q2,
    input wire Q3,
    input wire Q4
);

    or(A0, Q3, Q1);
    or(A1, Q2, Q3);
    
    assign A2 = Q4;

endmodule //encode_mod_5_to_3_bits

module SUM(
    output wire[2:0] Q,

    input  wire[2:0] A,
    input  wire[2:0] B
);
    wire A_Q0, A_Q1, A_Q2, A_Q3, A_Q4;
    wire B_Q0, B_Q1, B_Q2, B_Q3, B_Q4;

    decode_3_bits_to_mod_5 decA(
        .Q0(A_Q0),
        .Q1(A_Q1),
        .Q2(A_Q2),
        .Q3(A_Q3),
        .Q4(A_Q4),

        .A0(A[0]),
        .A1(A[1]),
        .A2(A[2])
    );

    decode_3_bits_to_mod_5 decB(
        .Q0(B_Q0),
        .Q1(B_Q1),
        .Q2(B_Q2),
        .Q3(B_Q3),
        .Q4(B_Q4),

        .A0(B[0]),
        .A1(B[1]),
        .A2(B[2])
    );

    wire S0_elem0, S0_elem1, S0_elem2, S0_elem3, S0_elem4;
    wire S1_elem0, S1_elem1, S1_elem2, S1_elem3, S1_elem4;
    wire S2_elem0, S2_elem1, S2_elem2, S2_elem3, S2_elem4;
    wire S3_elem0, S3_elem1, S3_elem2, S3_elem3, S3_elem4;
    wire S4_elem0, S4_elem1, S4_elem2, S4_elem3, S4_elem4;

    wire S_Q0, S_Q1, S_Q2, S_Q3, S_Q4;

    //SUM = 0 (mod 5)
    and(S0_elem0, A_Q0, B_Q0);
    and(S0_elem1, A_Q1, B_Q4);
    and(S0_elem2, A_Q2, B_Q3);
    and(S0_elem3, A_Q3, B_Q2);
    and(S0_elem4, A_Q4, B_Q1);
    or(S_Q0, S0_elem0, S0_elem1, S0_elem2, S0_elem3, S0_elem4);

    //SUM = 1 (mod 5)
    and(S1_elem0, A_Q0, B_Q1);
    and(S1_elem1, A_Q1, B_Q0);
    and(S1_elem2, A_Q2, B_Q4);
    and(S1_elem3, A_Q3, B_Q3);
    and(S1_elem4, A_Q4, B_Q2);
    or(S_Q1, S1_elem0, S1_elem1, S1_elem2, S1_elem3, S1_elem4);

    //SUM = 2 (mod 5)
    and(S2_elem0, A_Q0, B_Q2);
    and(S2_elem1, A_Q1, B_Q1);
    and(S2_elem2, A_Q2, B_Q0);
    and(S2_elem3, A_Q3, B_Q4);
    and(S2_elem4, A_Q4, B_Q3);
    or(S_Q2, S2_elem0, S2_elem1, S2_elem2, S2_elem3, S2_elem4);

    //SUM = 3 (mod 5)
    and(S3_elem0, A_Q0, B_Q3);
    and(S3_elem1, A_Q1, B_Q2);
    and(S3_elem2, A_Q2, B_Q1);
    and(S3_elem3, A_Q3, B_Q0);
    and(S3_elem4, A_Q4, B_Q4);
    or(S_Q3, S3_elem0, S3_elem1, S3_elem2, S3_elem3, S3_elem4);

    //SUM = 4 (mod 5)
    and(S4_elem0, A_Q0, B_Q4);
    and(S4_elem1, A_Q1, B_Q3);
    and(S4_elem2, A_Q2, B_Q2);
    and(S4_elem3, A_Q3, B_Q1);
    and(S4_elem4, A_Q4, B_Q0);
    or(S_Q4, S4_elem0, S4_elem1, S4_elem2, S4_elem3, S4_elem4);

    encode_mod_5_to_3_bits enc(
        .A0(Q[0]),
        .A1(Q[1]),
        .A2(Q[2]),
        
        .Q0(S_Q0),
        .Q1(S_Q1),
        .Q2(S_Q2),
        .Q3(S_Q3),
        .Q4(S_Q4)
    );
endmodule //SUM

module MEM(
    output wire[3:0] res,

    input wire[1:0] Cmd,
    input wire C,
    input wire[3:0] D,
    input wire RESET
);

    not(not_cmd_el_1, Cmd[1]);
    and(cmd_eq_Q01, Cmd[0], not_cmd_el_1);
    and(write_flag, cmd_eq_Q01, C);

    D_Trigger first_cell(
        .Q(res[0]),

        .C(write_flag),
        .D(D[0]),
        .RESET(RESET)
    );

    D_Trigger second_cell(
        .Q(res[1]),

        .C(write_flag),
        .D(D[1]),
        .RESET(RESET)
    );

    D_Trigger third_cell(
        .Q(res[2]),

        .C(write_flag),
        .D(D[2]),
        .RESET(RESET)
    );

    D_Trigger fourth_cell(
        .Q(res[3]),

        .C(write_flag),
        .D(D[3]),
        .RESET(RESET)
    );

endmodule //MEM

module D_Trigger_Front_Sync(
    output wire Q,
    output wire NQ,

    input wire D,
    input wire C,
    input wire RESET
);

    not(NC, C);
    wire tmp_Q;

    D_Trigger tmp(
        .Q(tmp_Q),

        .C(C),
        .D(D),
        .RESET(RESET)
    );

    D_Trigger fin(
        .Q(Q),
		.NQ(NQ),
        
        .C(NC),
        .D(tmp_Q),
        .RESET(RESET)
    );
endmodule //D_Trigger_Front_Sync

module counter(
    output wire[2:0] res,

    input wire[1:0] Cmd,
    input wire C,
    input wire[2:0] D,
    input wire RESET
);

    not(not_cmd_el_1, Cmd[1]);
    and(cmd_eq_Q01, Cmd[0], not_cmd_el_1);
    and(write_flag, cmd_eq_Q01, C);

    D_Trigger_Front_Sync first_cell(
        .Q(res[0]),

        .C(write_flag),
        .D(D[0]),
        .RESET(RESET)
    );

    D_Trigger_Front_Sync second_cell(
        .Q(res[1]),

        .C(write_flag),
        .D(D[1]),
        .RESET(RESET)
    );

    D_Trigger_Front_Sync third_cell(
        .Q(res[2]),

        .C(write_flag),
        .D(D[2]),
        .RESET(RESET)
    );
endmodule //counter

module head(
    output wire[2:0] Q,

    input wire[1:0] CMD,
    input wire CLK,
    input wire RESET
);

    not(NOT_CMD_Q0, CMD[0]);
    not(NOT_CMD_Q1, CMD[1]);
    and(is_cmd_01, CMD[0], NOT_CMD_Q1);
    and(is_cmd_10, NOT_CMD_Q0, CMD[1]);

    wire const_zero;
    assign const_zero = 1'b0;

    wire[1:0] two_bit_const_one;
    assign two_bit_const_one = 2'b01;

    wire[2:0] A;
    assign A = {is_cmd_10, const_zero, is_cmd_01};

    wire[2:0] sum_Q;

    counter cnt(
        .res(Q),

        .Cmd(two_bit_const_one),
        .C(CLK),
        .D(sum_Q),
        .RESET(RESET)
    );

    SUM sum_A_B(
        .Q(sum_Q),

        .A(A),
        .B(Q)
    );
endmodule //head

module and_1_4_bits(
    output wire[3:0] Q,

    input wire A, 
    input wire[3:0] D
);

    and (Q[0], D[0], A);
    and (Q[1], D[1], A);
    and (Q[2], D[2], A);
    and (Q[3], D[3], A);
endmodule //and_1_4_bits

module or_five_4bits(
    output wire[3:0] Q,

    input wire[3:0] D0,
    input wire[3:0] D1,
    input wire[3:0] D2,
    input wire[3:0] D3,
    input wire[3:0] D4
);
    or (Q[0], D0[0], D1[0], D2[0], D3[0], D4[0]);
    or (Q[1], D0[1], D1[1], D2[1], D3[1], D4[1]);
    or (Q[2], D0[2], D1[2], D2[2], D3[2], D4[2]);
    or (Q[3], D0[3], D1[3], D2[3], D3[3], D4[3]);
endmodule // or_five_4bits

module mux_get_MEM(
    output wire[3:0] Q,

    input wire A0,
    input wire A1,
    input wire A2,
    input wire[3:0] D0,
    input wire[3:0] D1,
    input wire[3:0] D2,
    input wire[3:0] D3,
    input wire[3:0] D4
);
    wire Q0, Q1, Q2, Q3, Q4;

    decode_3_bits_to_mod_5 decoder(
        .Q0(Q0),
        .Q1(Q1),
        .Q2(Q2),
        .Q3(Q3),
        .Q4(Q4),

        .A0(A0),
        .A1(A1),
        .A2(A2)
    );

    wire[3:0] and0_out, and1_out, and2_out, and3_out, and4_out;

    and_1_4_bits and0(.Q(and0_out), .A(Q0), .D(D0));
    and_1_4_bits and1(.Q(and1_out), .A(Q1), .D(D1));
    and_1_4_bits and2(.Q(and2_out), .A(Q2), .D(D2));
    and_1_4_bits and3(.Q(and3_out), .A(Q3), .D(D3));
    and_1_4_bits and4(.Q(and4_out), .A(Q4), .D(D4));

    or_five_4bits or_comb(
        .Q(Q),

        .D0(and0_out),
        .D1(and1_out),
        .D2(and2_out),
        .D3(and3_out),
        .D4(and4_out)
    );
endmodule // mux_get_MEM


module SUB(
    output wire[2:0] Q,
    
    input wire[2:0] A,
    input wire[2:0] B
);
    wire ZERO;
    assign ZERO = 1'b0;

    wire[2:0] sum_0_B;
    assign sum_0_B = {B[0], ZERO, ZERO};

    wire[2:0] sum_Q0;
    SUM sum_0(
        .Q(sum_Q0),
        
        .A(A),
        .B(sum_0_B)
    );

    wire[2:0] sum_1_B;
    assign sum_1_B = {ZERO, B[1], B[1]};

    wire[2:0] sum_Q1;
    SUM sum_1(
        .Q(sum_Q1),
        
        .A(sum_Q0),
        .B(sum_1_B)
    );

    wire[2:0] sum_2_B;
    assign sum_2_B = {ZERO, ZERO, B[2]};

    SUM sum_2(
        .Q(Q),
        
        .A(sum_Q1),
        .B(sum_2_B)
    );
endmodule // SUB

module stack(
    inout wire [3:0] IO_DATA,
    input wire CLK,
    input wire RESET,
    input wire [1:0] COMMAND,
    input wire [2:0] INDEX
);

    wire [2:0] head_Q;
    head stack_head(
        .Q(head_Q),

        .CMD(COMMAND),
        .CLK(CLK),
        .RESET(RESET)
    );

    wire [2:0] tmp_sub_Q;
    and(is_cmd_11, COMMAND[0], COMMAND[1]);
    and(tmp_sub_B_0, INDEX[0], is_cmd_11);
    and(tmp_sub_B_1, INDEX[1], is_cmd_11);
    and(tmp_sub_B_2, INDEX[2], is_cmd_11);
    wire[2:0] tmp_sub_B_in;
    assign tmp_sub_B_in = {tmp_sub_B_2, tmp_sub_B_1, tmp_sub_B_0};
    SUB tmp_sub(
        .Q(tmp_sub_Q),

        .A(head_Q),
        .B(tmp_sub_B_in)
    );

    wire [2:0] sub_Q;
    not(NCMD_0, COMMAND[0]);
    and(is_cmd_10, NCMD_0, COMMAND[1]);
    or(is_cmd_10_or_11, is_cmd_10, is_cmd_11);
    wire[2:0] sub_B_in;
    assign sub_B_in = {1'b0, 1'b0, is_cmd_10_or_11};
    SUB sub(
        .Q(sub_Q),

        .A(tmp_sub_Q),
        .B(sub_B_in)
    );

    wire Q0, Q1, Q2, Q3, Q4;
    decode_3_bits_to_mod_5 decoder_head(
        .Q0(Q0),
        .Q1(Q1),
        .Q2(Q2),
        .Q3(Q3),
        .Q4(Q4),

        .A0(head_Q[0]),
        .A1(head_Q[1]),
        .A2(head_Q[2])
    );

    wire C_MEM0, C_MEM1, C_MEM2, C_MEM3, C_MEM4;
    and(C_MEM0, CLK, Q0);
    and(C_MEM1, CLK, Q1);
    and(C_MEM2, CLK, Q2);
    and(C_MEM3, CLK, Q3);
    and(C_MEM4, CLK, Q4);

    wire[3:0] MEM0_out, MEM1_out, MEM2_out, MEM3_out, MEM4_out;
    MEM mem0(
        .res(MEM0_out),

        .Cmd(COMMAND),
        .C(C_MEM0),
        .D(IO_DATA),
        .RESET(RESET)
    );

    MEM mem1(
        .res(MEM1_out),

        .Cmd(COMMAND),
        .C(C_MEM1),
        .D(IO_DATA),
        .RESET(RESET)
    );

    MEM mem2(
        .res(MEM2_out),

        .Cmd(COMMAND),
        .C(C_MEM2),
        .D(IO_DATA),
        .RESET(RESET)
    );

    MEM mem3(
        .res(MEM3_out),

        .Cmd(COMMAND),
        .C(C_MEM3),
        .D(IO_DATA),
        .RESET(RESET)
    );

    MEM mem4(
        .res(MEM4_out),

        .Cmd(COMMAND),
        .C(C_MEM4),
        .D(IO_DATA),
        .RESET(RESET)
    );

    wire[3:0] mem_data_head;
    wire[3:0] mem_data_index;
    mux_get_MEM mem_mux_head(
        .Q(mem_data_head),

        .A0(sub_Q[0]),
        .A1(sub_Q[1]),
        .A2(sub_Q[2]),
        .D0(MEM0_out),
        .D1(MEM1_out),
        .D2(MEM2_out),
        .D3(MEM3_out),
        .D4(MEM4_out)
    );

    wire[3:0] out;
    wire[1:0] const_cmd_01;
    assign const_cmd_01 = 2'b01;
    MEM mem_res(
        .res(out),

        .Cmd(const_cmd_01),
        .C(CLK),
        .D(mem_data_head),
        .RESET(RESET)
    );

    and(is_cmd_get, COMMAND[0], COMMAND[1]);
    not(not_cmd_0, COMMAND[0]);
    and(is_cmd_pop, not_cmd_0, COMMAND[1]);
    or(is_cmd_get_or_pop, is_cmd_get, is_cmd_pop);
    and(is_cmd_get_or_pop_and_clocked, is_cmd_get_or_pop, CLK);
    not(not_is_cmd_get_or_pop_and_clocked, is_cmd_get_or_pop_and_clocked);
    pmos(IO_DATA[0], out[0], not_is_cmd_get_or_pop_and_clocked);
    pmos(IO_DATA[1], out[1], not_is_cmd_get_or_pop_and_clocked);
    pmos(IO_DATA[2], out[2], not_is_cmd_get_or_pop_and_clocked);
    pmos(IO_DATA[3], out[3], not_is_cmd_get_or_pop_and_clocked);
    nmos(IO_DATA[0], out[0], is_cmd_get_or_pop_and_clocked);
    nmos(IO_DATA[1], out[1], is_cmd_get_or_pop_and_clocked);
    nmos(IO_DATA[2], out[2], is_cmd_get_or_pop_and_clocked);
    nmos(IO_DATA[3], out[3], is_cmd_get_or_pop_and_clocked);

endmodule // stack