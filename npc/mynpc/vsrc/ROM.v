module ROM(
    input [31:0] address,
    output [31:0] inst
);

    reg [31:0] memory [1023:0];

    // initial begin
    //     memory[0] = 32'h00000513; // addi x10, x0, 0
    //     memory[1] = 32'h04800593; // addi x11, x0, 72
    //     memory[2] = 32'h00100073; // ebreak
    // end

    initial begin
        $readmemh("/home/yanxy/ysyx/ysyx-workbench/npc/mynpc/vsrc/inst.hex",memory);
        memory[0] = 32'h00000513; // addi x10, x0, 0
        memory[1] = 32'h04800593; // addi x11, x0, 72
        memory[12] = 32'h00100073; // ebreak

        // integer i;
        // for(i=0;i<10;i=i+1) begin
        //     $$display("memory[$d]=%h",i,memory[i]);
        // end
    end


    assign inst=memory[address>>2];

endmodule

