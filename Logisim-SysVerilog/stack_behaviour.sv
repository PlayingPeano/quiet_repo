module stack_behaviour_normal(
    inout wire[3:0] IO_DATA, 
    input wire RESET, 
    input wire CLK, 
    input wire[1:0] COMMAND,
    input wire[2:0] INDEX
    ); 
    
    reg [3:0] mem [0:4];
    reg [2:0] head;
    reg [3:0] data_out;
    reg output_flag;
    initial output_flag = 0;

    assign IO_DATA = (output_flag == 1 && CLK) ? data_out : 4'bzzzz;

    always @(posedge CLK or posedge RESET) begin
        if (RESET) begin
            head <= 0;
            for (int i = 0; i < 5; ++i) mem[i] <= 0;
            output_flag <= 0;
        end else begin
            case (COMMAND)
                2'b01: begin
                    mem[head] <= IO_DATA;
                    head <= (head + 1) % 5;
                    output_flag <= 0;
                end
                2'b10: begin
                    data_out <= mem[(head - 1 + 5) % 5];
                    head <= (head - 1 + 5) % 5;
                    output_flag <= 1;
                end
                2'b11: begin
                    data_out <= mem[(head - INDEX - 1 + 10) % 5];
                    output_flag <= 1;
                end
                default: output_flag <= 0;
            endcase
        end
    end
endmodule