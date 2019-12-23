module blink (
    // Horloge
    input clock,
    output led
);

// Icestick clock : 12Mhz
//parameter clock_freq = 12_000_000;
// LicheeTang clock : 24Mhz
//parameter clock_freq = 24_000_000;
// Gowin 100Mhz
parameter clock_freq = 100_000_000;

localparam MAX_COUNT = clock_freq;
localparam MAX_COUNT_UPPER = $clog2(MAX_COUNT);

reg [MAX_COUNT_UPPER:0] counter;
reg led_reg;

assign led = led_reg;

always@(posedge clock)
begin
    if(counter < MAX_COUNT/2)
        led_reg <= 1;
    else
        led_reg <= 0;

    if(counter >= MAX_COUNT)
        counter <= 0;
    else
        counter <= counter + 1;
end

endmodule
