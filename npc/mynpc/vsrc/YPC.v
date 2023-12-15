module YPC (
    input clk,
    input reset,
    output reg halt
);

    wire [31:0] pc;
    reg [31:0] next_pc;

    ProgramCounter pc_reg(
        .clk(clk),
        .reset(reset),
        .pc(pc),
        .next_pc(next_pc)
    );

    // 寄存器文件接口
    wire [31:0] rs1_data;
    wire [4:0] rs1_addr, rd_addr;
    wire [31:0] imm;
    wire regWriteEn;
    wire aluop;
    wire isbreak;

    reg [31:0] rd_data;

    // 寄存器文件实例化
    RegisterFile #(5, 32) R (
        .clk(clk),
        .wdata(rd_data),
        .waddr(rd_addr),
        .wen(regWriteEn),
        .raddr1(rs1_addr),
        .rdata1(rs1_data)
    );

    // 指令存储器
    reg [31:0] inst;

    ROM inst_mem(
        .address(pc),
        .inst(inst)
    );

    //译码单元
    InstDecoder decoder(
        .inst(inst),
        .rs1(rs1_addr),
        .rd(rd_addr),
        .imm(imm),
        .aluop(aluop),
        .isbreak(isbreak),
        .regWriteEn(regWriteEn)
    );

    // 执行指令
    always @(posedge clk) begin
        if (reset) begin
            halt <= 0;
        end else begin
            if (isbreak) begin
                halt <= 1; // 停机
            end else begin
                if (aluop) begin
                    rd_data<=rs1_data+imm;
                end
                next_pc <= pc + 4; // 更新 PC
            end
        end
    end
endmodule
