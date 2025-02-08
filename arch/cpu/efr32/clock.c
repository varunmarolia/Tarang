#include "clock.h"
#include "atomic-arch.h"

static volatile clock_time_t clock_ticks = 0;
static uint32_t usecond_clocks_10X;  /* 10 x number of system clocks required for 1 usecond */
/*---------------------------------------------------------------------------*/
void SysTick_Handler(void) __attribute__ ((interrupt));
void
SysTick_Handler(void)
{
  ATOMIC_SECTION(clock_ticks++;);
}
/*---------------------------------------------------------------------------*/
void 
clock_init(void)
{
  uint32_t sys_clk = SystemCoreClockGet();  /* system clock freq in HZ */
  /* board specfic HFXO crystal should be selected and enabled in the board_init function under board.d files */
  /* set system tick to generate interrupt at 1ms. Accuracy depends upon crystal tune */
  SysTick_Config(sys_clk / CLOCK_TICKS_CONF);
  usecond_clocks_10X = sys_clk / 100000; 
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_get_ticks(void)
{
  return clock_ticks;
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_get_time_ms(void)
{
  return ((clock_ticks * 1000) / CLOCK_TICKS_CONF);
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_get_seconds(void)
{
  return (clock_get_time_ms() / 1000);
}
/*---------------------------------------------------------------------------*/
void
clock_wait_ms(clock_time_t time_ms)
{
  clock_time_t start;
  start = clock_get_time_ms();
  while(clock_get_time_ms() - start < time_ms); 
}
/*---------------------------------------------------------------------------*/
void
clock_delay_us(uint32_t time_us)
{
  /* calcualte number of loops required for passing 1 us 
   * It takes roughly 5 cycles on ARM processor to down count to zero
   * remove approximate 20 cycles for calcualtions
   */
  uint32_t loops = ((usecond_clocks_10X - 20) * time_us * 5) / 10;
    while(loops > 0) {
      loops--;
    }
}
/*---------------------------------------------------------------------------*/
