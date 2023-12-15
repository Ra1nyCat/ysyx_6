module RegisterFile #(parameter ADDR_WIDTH = 5, DATA_WIDTH = 32) (
    input clk,
    input [DATA_WIDTH-1:0] wdata,
    input [ADDR_WIDTH-1:0] waddr,
    input wen,
    input [ADDR_WIDTH-1:0] raddr1,
    output [DATA_WIDTH-1:0] rdata1
);
    reg [DATA_WIDTH-1:0] rf [0:2**ADDR_WIDTH-1];

    always @(posedge clk) begin
        if (wen && waddr != 5'd0) rf[waddr] <= wdata;
    end

    assign rdata1 = (raddr1 !=5'd0) ? rf[raddr1] : 0;
endmodule
