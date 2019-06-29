#include "pmsis.h"
#include "chips/gap8/drivers/pin_names.h"
#include "chips/gap8/drivers/pin_config.h"

/* Array of PORT peripheral base address. */
static PORT_Type *const port_addrs[] = PORT_BASE_PTRS;
static GPIO_Type *const gpio_addrs[] = GPIO_BASE_PTRS;

void pin_function(pin_mode_e pin, int function)
{
    int pin_num = (pin & 0xFF) - GAP_PIN_OFFSET;

    if (0<= pin_num && pin_num < GAP_PORT_PIN_NUM )
        port_set_pin_mux(port_addrs[GET_GPIO_PORT(pin)], pin_num, (port_mux_t)function);
}

void pin_mode(pin_name_e pin, pin_mode_e mode)
{

    uint32_t is_gpio = GET_IS_GPIO(pin);
    uint32_t instance = GET_GPIO_PORT(pin);

    if(is_gpio) {
        uint32_t gpio_num = GET_GPIO_NUM(pin);

        int reg_num = gpio_num >> 2;
        int pos = (gpio_num & 0x3) << 3;

        GPIO_Type *base = gpio_addrs[instance];
        switch (mode) {
        case PULL_NONE:
            /* Write 0 to the PullUp and PullDown Enable bits */
#if defined(__GAP8__)
            base->PADCFG[reg_num] &= ~(1U << pos);
            break;
        case PULL_UP:
            /* Write 1 to the PullUp Enable bits */
            base->PADCFG[reg_num] |= (1U << pos);
            break;
#elif defined(__GAP9__)
            base->PADCFG[reg_num] &= ~(3U << pos);
            break;
        case PULL_DOWN:
            /* Write 1 to the PD Enable bits */
            base->PADCFG[reg_num] |= (1U << pos);
            break;
        case PULL_UP:
            /* Write 1 to the PU Enable bits */
            base->PADCFG[reg_num] |= (2U << pos);
            break;
#endif
        default:
            break;
        }
    } else {
        int pin_num = (pin & 0xFF) - GAP_PIN_OFFSET;

        if (0 <= pin_num && pin_num < GAP_PORT_PIN_NUM ) {
            int reg_num = pin_num >> 2;
            int pos = (pin_num & 0x3) << 3;

            PORT_Type *base = port_addrs[instance];

            switch (mode) {
            case PULL_NONE:
                /* Write 0 to the PullUp and PullDown Enable bits */
#if defined(__GAP8__)
                base->PADCFG[reg_num] &= ~(1U << pos);
                break;
            case PULL_UP:
                /* Write 1 to the PullUp Enable bits */
                base->PADCFG[reg_num] |= (1U << pos);
                break;
#elif defined(__GAP9__)
                base->PADCFG[reg_num] &= ~(3U << pos);
                break;
            case PULL_DOWN:
                /* Write 1 to the PD Enable bits */
                base->PADCFG[reg_num] |= (1U << pos);
                break;
            case PULL_UP:
                /* Write 1 to the PU Enable bits */
                base->PADCFG[reg_num] |= (2U << pos);
                break;
#endif
            default:
                break;
            }
        }
    }
}
