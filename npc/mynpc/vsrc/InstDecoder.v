module InstDecoder(
    input [31:0] inst,
    output reg [4:0] rs1,
    output reg [4:0] rd,
    output reg [31:0] imm,
    output reg  aluop,
    output reg  isbreak,
    output reg  regWriteEn
);

wire isaddi;
assign isaddi=(inst[6:0]==7'b0010011)&&(inst[14:12]==3'b000);

always @(*) begin
    rs1 = isaddi ? inst[19:15] : 5'b0;
    rd = isaddi ? inst[11:7] : 5'b0;
    imm = isaddi ? {{20{inst[31]}}, inst[31:20]} : 32'b0;
    isbreak = (inst == 32'h00100073);
    aluop = isaddi ? 1'b1 : 1'b0;
    regWriteEn = isaddi ? 1'b1 : 1'b0;
end

endmodule
