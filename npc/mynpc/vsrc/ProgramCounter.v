module ProgramCounter(
    input clk,
    input reset,
    input [31:0] next_pc,
    output reg [31:0] pc
);

    //设置pc的初始值
    initial begin
        pc = 32'h0;
    end

    //在时钟上升沿时更新pc或者重置pc
    always @(posedge clk) begin
        if (reset) begin
            pc <= 32'h0;
        end else begin
            pc <= next_pc;
        end
    end

endmodule

