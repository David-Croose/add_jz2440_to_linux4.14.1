#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/console.h>
#include <linux/kernel.h>
#include <linux/irqdomain.h>
#include <mach/virt_addr.h>
#include <mach/hardware.h>
#include <mach/clock.h>
#include <asm/io.h>
#include <linux/tty_flip.h>

#define QIN2440_TOTAL_PORTS    3

#define UART_WRITE_REG(val, reg, tail, port)                      \
	__raw_write##tail(val, reg##0 - (unsigned int)__ULCON0 + port->mapbase);

#define UART_READ_REG(reg, tail, port)                            \
	__raw_read##tail(reg##0 - (unsigned int)__ULCON0 + port->mapbase)

#define S3C2410_UERSTAT_OVERRUN	  (1<<0)
#define S3C2410_UERSTAT_FRAME	  (1<<2)
#define S3C2410_UERSTAT_BREAK	  (1<<3)
#define S3C2410_UERSTAT_PARITY	  (0x1000)
#define S3C2410_UERSTAT_ANY	  (S3C2410_UERSTAT_OVERRUN | \
							   S3C2410_UERSTAT_FRAME   | \
							   S3C2410_UERSTAT_BREAK)
/* flag to ignore all characters comming in */
#define RXSTAT_DUMMY_READ (0x10000000)

/*
 *	Local per-uart structure.
 */
struct qin2440_uart {
	struct uart_port	port;
	char				txirq_enable;
	char				first_tx_flag;
};

static const struct uart_ops qin2440_uart_ops;

static struct qin2440_uart qin2440_ports[QIN2440_TOTAL_PORTS] = {
	[0] = {
		.port = {
			.line		= 0,
			.fifosize	= 64,
			.mapbase	= (unsigned int)__ULCON0,
			.membase	= __UTXH0,
			.iotype		= UPIO_MEM,
			/// .irq		= IRQ_UART0_RXD,
			.uartclk	= 50000000,
			.ops		= &qin2440_uart_ops,
			.flags		= ASYNC_BOOT_AUTOCONF,
		},
		.txirq_enable = 0,
	},
#if (QIN2440_TOTAL_PORTS == 2 || QIN2440_TOTAL_PORTS == 3)
	[1] = {
		.port = {
			.line		= 1,
			.fifosize	= 64,
			.mapbase	= (unsigned int)__ULCON1,
			.membase	= __UTXH1,
			.iotype		= UPIO_MEM,
			/// .irq		= IRQ_UART1_RXD,
			.uartclk	= 50000000,
			.ops		= &qin2440_uart_ops,
			.flags		= ASYNC_BOOT_AUTOCONF,
		},
		.txirq_enable = 0,
	},
#endif
#if (QIN2440_TOTAL_PORTS == 3)
	[2] = {
		.port = {
			.line		= 2,
			.fifosize	= 64,
			.mapbase	= (unsigned int)__ULCON2,
			.membase	= __UTXH2,
			.iotype		= UPIO_MEM,
			/// .irq		= IRQ_UART2_RXD,
			.uartclk	= 50000000,
			.ops		= &qin2440_uart_ops,
			.flags		= ASYNC_BOOT_AUTOCONF,
		},
		.txirq_enable = 0,
	},
#endif
};

static irqreturn_t qin2440_tx_chars(int irq, void *dev_id);

static void uart_txirq_enable(struct uart_port *port)
{
	struct qin2440_uart *parent_port = container_of(port, struct qin2440_uart, port);

	/*
	 * TODO
	 * Call an irq-handler by a user routine is not a good solution
	 *
	 * Maybe s3c2440 is different from others SOC, but it truely need
	 * send a byte to trigger a tx irq, While the others SOC just need
	 * enabling the tx irq to trigger tx irq.
	 *
	 * Why the "tx_chars()" just run one time in "tx_start()" runs for
	 * the fitst time? I don't know, I just put them here and they work
	 * well
	 */

	if(port->line == 0) {
		if(parent_port->first_tx_flag == 0) {
			qin2440_tx_chars(port->irq + 1, port);
			parent_port->first_tx_flag = 1;
		}
	} else {
		// TODO, why uart1 and uart2 don't need the "fist run" limitation? I don't know
		qin2440_tx_chars(port->irq + 1, port);
	}

	if(parent_port->txirq_enable == 0) {
		enable_irq(port->irq + 1);
		parent_port->txirq_enable = 1;
	}
}

static void uart_txirq_disable(struct uart_port *port)
{
 	struct qin2440_uart *parent_port = container_of(port, struct qin2440_uart, port);

	if(parent_port->txirq_enable == 1) {
		disable_irq(port->irq + 1);
		parent_port->txirq_enable = 0;
	}
}

static void uart_rxirq_disable(struct uart_port *port)
{
	disable_irq(port->irq);
}

static irqreturn_t qin2440_rx_chars(int irq, void *dev_id)
{
	struct uart_port *port = dev_id;
	unsigned int ch, ufcon, flag, ufstat, uerstat;
	int max_count = 64;

	while(max_count-- > 0) {
		ufcon = UART_READ_REG(__UFCON, l, port);
		ufstat = UART_READ_REG(__UFSTAT, l, port);

		/* if(UART_READ_REG(__UFCON, l, port) & 1) */ {    // if it is fifo mode
			if((ufstat & 0x3F) == 0) {    // if the rx fifo is empty
				break;
			}
		}

		uerstat = UART_READ_REG(__UERSTAT, l, port);
		ch = UART_READ_REG(__URXH, b, port);

		/* insert the character into the buffer */
		flag = TTY_NORMAL;
		port->icount.rx++;

		if(uerstat & S3C2410_UERSTAT_ANY) {
			/* check for break */
			if(uerstat & S3C2410_UERSTAT_BREAK) {
				port->icount.brk++;
				if(uart_handle_break(port)) {
				    goto ignore_char;
				}
			}

			if(uerstat & S3C2410_UERSTAT_FRAME) {
				port->icount.frame++;
			}
			if(uerstat & S3C2410_UERSTAT_OVERRUN) {
				port->icount.overrun++;
			}

			uerstat &= port->read_status_mask;

			if(uerstat & S3C2410_UERSTAT_BREAK) {
				flag = TTY_BREAK;
			}
			else if(uerstat & S3C2410_UERSTAT_PARITY) {
				flag = TTY_PARITY;
			}
			else if(uerstat & ( S3C2410_UERSTAT_FRAME | S3C2410_UERSTAT_OVERRUN)) {
				flag = TTY_FRAME;
			}
		}

		if(uart_handle_sysrq_char(port, ch)) {
			goto ignore_char;
		}

		uart_insert_char(port, uerstat, S3C2410_UERSTAT_OVERRUN, ch, flag);

		/// break;   // if you set fifo=1, you will need this

ignore_char:
		continue;
	}
	tty_flip_buffer_push(&port->state->port);

	return IRQ_HANDLED;
}

static irqreturn_t qin2440_tx_chars(int irq, void *dev_id)
{
	struct uart_port *port = dev_id;
	struct circ_buf *xmit = &port->state->xmit;
	int count = 256;

	if(port->x_char) {
		__raw_writeb(port->x_char, port->membase);

		// this printk can't be delete, or the uart1 and uart2 works bad
		if(port->line != 0) {
			printk(KERN_INFO "*");
		}

		port->icount.tx++;
		port->x_char = 0;
		goto out;
	}

	if(uart_circ_empty(xmit) || uart_tx_stopped(port)) {
		uart_txirq_disable(port);
		goto out;
	}

	while(!uart_circ_empty(xmit) && count-- > 0) {
		if(UART_READ_REG(__UFSTAT, l, port) & (1 << 14)) {
			break;
		}
		__raw_writeb(xmit->buf[xmit->tail], port->membase);

		// this printk can't be delete, or the uart1 and uart2 works bad
		if(port->line != 0) {
			printk(KERN_INFO "*");
		}

		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;

		break;  // whatever the fifo equals to what, you always need this line, I don't know
	}

	if(uart_circ_chars_pending(xmit) < WAKEUP_CHARS) {
		uart_write_wakeup(port);
	}

	if(uart_circ_empty(xmit)) {
		uart_txirq_disable(port);
	}

out:
	return IRQ_HANDLED;
}

static unsigned int qin2440_tx_empty(struct uart_port *port)
{
	unsigned int tmp;

	if(UART_READ_REG(__UFCON, l, port) & 1) {  	// if this is fifo mode
		tmp = UART_READ_REG(__UFSTAT, l, port);
		if(tmp & (1 << 14)    			// if tx fifo is full
	       || tmp & (0x3F << 8)) {      // if tx fifo has bytes but not full
			return 0;
		}
		return 1;
	}

	return (UART_READ_REG(__UTRSTAT, l, port) & (1 << 2));
}

static void qin2440_start_tx(struct uart_port *port)
{
	uart_txirq_enable(port);
}

static void qin2440_stop_tx(struct uart_port *port)
{
	uart_txirq_disable(port);
}

static void qin2440_stop_rx(struct uart_port *port)
{
	uart_rxirq_disable(port);
}

static void qin2440_break_ctl(struct uart_port *port, int break_state)
{
	unsigned long flags;
	unsigned int tmp;

	spin_lock_irqsave(&port->lock, flags);

	tmp = UART_READ_REG(__UCON, l, port);
	if(break_state) {
		tmp |= 1 << 4;
	} else {
		tmp &= ~(1 << 4);
	}
	UART_WRITE_REG(tmp, __UCON, l, port);

	spin_unlock_irqrestore(&port->lock, flags);
}

static int qin2440_startup(struct uart_port *port)
{
	struct qin2440_uart *parent_port = container_of(port, struct qin2440_uart, port);
	int ret;
	unsigned int virq;

	switch (port->line) {
	case 0:
		virq = irq_find_mapping(irq_submisc.domain, IRQ_UART0_RXD);
		break;
	case 1:
		virq = irq_find_mapping(irq_submisc.domain, IRQ_UART1_RXD);
		break;
	case 2:
		virq = irq_find_mapping(irq_submisc.domain, IRQ_UART2_RXD);
		break;
	default:
		return -1;
	};

	ret = request_irq(port->irq, qin2440_rx_chars,
					  0, "qin2440_uart_rxirq", port);
	if(ret != 0) {
		printk(KERN_ERR "qin2440_startup: request rxirq(%d) failed\n", port->line);
		return ret;
	}

	ret = request_irq(port->irq + 1, qin2440_tx_chars,
					  0, "qin2440_uart_txirq", port);
	if(ret != 0) {
		printk(KERN_ERR "qin2440_startup: request txirq(%d) failed\n", port->line);
		return ret;
	}

	parent_port->txirq_enable = 1;

	return 0;
}

static void qin2440_shutdown(struct uart_port *port)
{
	free_irq(port->irq, port);
	free_irq(port->irq + 1, port);
}

/**
 * init uart gpio multiplexing
 *
 * @param port: the uart port
 */
static void uart_gpio_init(struct uart_port *port)
{
	unsigned int tmp;

	switch(port->line) {
	case 0:
		// set GPH2 multiplexed as TX0
		tmp = __raw_readl(__GPHCON);
		tmp &= ~((0x3 & 0x3) << (2 * 2));
		tmp |= (0x2 & 0x3) << (2 * 2);
		__raw_writel(tmp, __GPHCON);

		// set GPH3 multiplexed as RX0
		tmp = __raw_readl(__GPHCON);
		tmp &= ~((0x3 & 0x3) << (3 * 2));
		tmp |= (0x2 & 0x3) << (3 * 2);
		__raw_writel(tmp, __GPHCON);

		// set GPH2 pull up
		tmp = __raw_readl(__GPHUP);
		tmp &= ~(1 << 2);
		__raw_writel(tmp, __GPHUP);

		// set GPH3 pull up
		tmp = __raw_readl(__GPHUP);
		tmp &= ~(1 << 3);
		__raw_writel(tmp, __GPHUP);
		break;

	case 1:
		// set GPH4 multiplexed as TX1
		tmp = __raw_readl(__GPHCON);
		tmp &= ~((0x3 & 0x3) << (4 * 2));
		tmp |= (0x2 & 0x3) << (4 * 2);
		__raw_writel(tmp, __GPHCON);

		// set GPH5 multiplexed as RX1
		tmp = __raw_readl(__GPHCON);
		tmp &= ~((0x3 & 0x3) << (5 * 2));
		tmp |= (0x2 & 0x3) << (5 * 2);
		__raw_writel(tmp, __GPHCON);

		// set GPH4 pull up
		tmp = __raw_readl(__GPHUP);
		tmp &= ~(1 << 4);
		__raw_writel(tmp, __GPHUP);

		// set GPH5 pull up
		tmp = __raw_readl(__GPHUP);
		tmp &= ~(1 << 5);
		__raw_writel(tmp, __GPHUP);
		break;

	case 2:
		// set GPH6 multiplexed as TX2
		tmp = __raw_readl(__GPHCON);
		tmp &= ~((0x3 & 0x3) << (6 * 2));
		tmp |= (0x2 & 0x3) << (6 * 2);
		__raw_writel(tmp, __GPHCON);

		// set GPH7 multiplexed as RX2
		tmp = __raw_readl(__GPHCON);
		tmp &= ~((0x3 & 0x3) << (7 * 2));
		tmp |= (0x2 & 0x3) << (7 * 2);
		__raw_writel(tmp, __GPHCON);

		// set GPH6 pull up
		tmp = __raw_readl(__GPHUP);
		tmp &= ~(1 << 6);
		__raw_writel(tmp, __GPHUP);

		// set GPH7 pull up
		tmp = __raw_readl(__GPHUP);
		tmp &= ~(1 << 7);
		__raw_writel(tmp, __GPHUP);
		break;

	default:
		printk(KERN_ERR "%s(line:%d): the \"uartnr\" given is bad!\n",
				__FUNCTION__, __LINE__);
	}
}

/**
 * uart baudrate, databit-length, stop bit init
 *
 * @param baud: the uart baudrate
 * @param c_cflag: control mode flags
 * @param port: the uart port
 */
static void uart_misc_init(unsigned int baud, unsigned int c_cflag,
						struct uart_port *port)
{
	unsigned int ulcon;
	unsigned int ucon;
	unsigned int ubrdiv;

	ulcon = 0;
	switch(c_cflag & CSIZE) {
	case CS5:
		ulcon |= (0 & 3) << 0;
		break;
	case CS6:
		ulcon |= (1 & 3) << 0;
		break;
	case CS7:
		ulcon |= (2 & 3) << 0;
		break;
	default:
	case CS8:
		ulcon |= (3 & 3) << 0;
		break;
	}

	if(c_cflag & CSTOPB) {
		ulcon |= (1 & 1 ) << 2;			// 2 stop bit
	} else {
		ulcon &= ~((1 & 1 ) << 2);			// 1 stop bit
	}

	if(c_cflag & PARENB) {
		if(c_cflag & PARODD) {
			ulcon |= (4 & 7) << 3;
		} else {
			ulcon |= (5 & 7) << 3;
		}
	} else {
		ulcon |= (0 & 7) << 3;
	}

	UART_WRITE_REG(ulcon, __ULCON, l, port);

	ucon = 0;
	ucon |= (1 & 0x3) << 0;    // Rx Interrupt request or polling mode
	ucon |= (1 & 0x3) << 2;    // Tx Interrupt request or polling mode
	ucon |= (0 & 0x1) << 4;    // Don't send break signal while transmitting
	ucon |= (0 & 0x1) << 5;    // Don't use loopback mode
	ucon |= (1 & 0x1) << 6;    // Generate receive error status interrupt
	ucon |= (1 & 0x1) << 7;    // Disable Rx time out interrupt when UART FIFO is enabled. The interrupt is a receive interrupt
	ucon |= (0 & 0x1) << 8;    // Interrupt is requested the instant Rx buffer receivesthe data in Non-FIFO mode or reaches Rx FIFO Trigger Level inFIFO mode
	ucon |= (0 & 0x1) << 9;    // Interrupt is requested as soon as the Tx bufferbecomes empty in Non-FIFO mode or reaches Tx FIFO TriggerLevel in FIFO mode
	ucon |= (0 & 0x3) << 10;   // Select PCLK as the source clock of UART0
	UART_WRITE_REG(ucon, __UCON, l, port);

	// UBRDIVn = (int)( UART clock / ( buad rate x 16) ) ¨C1
	ubrdiv = (port->uartclk * 10 / baud / 16 % 10) >= 5
				? (port->uartclk / baud / 16 + 1 - 1)
				: (port->uartclk / baud / 16 - 1);
	UART_WRITE_REG(ubrdiv, __UBRDIV, l, port);
}

/**
 * uart fifo init
 *
 * @param deep: the fifo deep
 *              0 --- 1 byte(no fifo)
 *              16 --- 16 bytes
 *              32 --- 32 bytes
 * @param port: the uart port
 */
static void uart_fifo_init(int deep, struct uart_port *port)
{
	unsigned int ufcon;

	// reset tx, rx fifo
	ufcon = UART_READ_REG(__UFCON, l, port);
	ufcon |= (3 << 1);
	UART_WRITE_REG(ufcon, __UFCON, l, port);

	ufcon = 0;
	switch(deep) {
	case 0:
		ufcon |= 0;							// disable fifo, thus the tx fifo=1, rx fifo=1
		break;

	case 16:
		ufcon |= (1 & 3) << 6;				// tx fifo=16
		ufcon |= (2 & 3) << 4;				// rx fifo=16
		break;

	case 32:
		ufcon |= (2 & 3) << 6;				// tx fifo=32
		ufcon |= (3 & 3) << 4;				// rx fifo=32
		break;

	default:
		ufcon |= 0;							// disable fifo, thus the tx fifo=1, rx fifo=1
		printk(KERN_ERR "%s(line:%d) error: you give a wrong deep(%d), replace with 0\n",
						__FUNCTION__, __LINE__, deep);
	}
	UART_WRITE_REG(ufcon, __UFCON, l, port);
}

static void qin2440_set_termios(struct uart_port *port,
								struct ktermios *termios,
								struct ktermios *old)
{
	unsigned int baud;
	unsigned long flags;

	/*
	 * We don't support modem control lines.
	 */
	termios->c_cflag &= ~(HUPCL | CMSPAR);
	termios->c_cflag |= CLOCAL;

	baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk / 16);

	spin_lock_irqsave(&port->lock, flags);

	// set uart gpio multiplexing
	uart_gpio_init(port);

	// set uart 8n1, baudrate, irq
	uart_misc_init(baud, termios->c_cflag, port);

	// set uart fifo
	uart_fifo_init(16, port);

	uart_update_timeout(port, termios->c_cflag, baud);

	/*
	 * Which character status flags are we interested in?
	 */
	port->read_status_mask = S3C2410_UERSTAT_OVERRUN;
	if(termios->c_iflag & INPCK) {
		port->read_status_mask |= S3C2410_UERSTAT_FRAME | S3C2410_UERSTAT_PARITY;
	}

	/*
	 * Which character status flags should we ignore?
	 */
	port->ignore_status_mask = 0;
	if(termios->c_iflag & IGNPAR) {
		port->ignore_status_mask |= S3C2410_UERSTAT_OVERRUN;
	}
	if(termios->c_iflag & IGNBRK && termios->c_iflag & IGNPAR) {
		port->ignore_status_mask |= S3C2410_UERSTAT_FRAME;
	}

	/*
	 * Ignore all characters if CREAD is not set.
	 */
	if((termios->c_cflag & CREAD) == 0) {
		port->ignore_status_mask |= RXSTAT_DUMMY_READ;
	}

	spin_unlock_irqrestore(&port->lock, flags);
}

static const char *qin2440_type(struct uart_port *port)
{
	return "qin2440_uart";    // any name you can put here
}

static int qin2440_request_port(struct uart_port *port)
{
	// nothing to do
	return 0;
}

static void qin2440_release_port(struct uart_port *port)
{
	// nothing to do
}

static void qin2440_config_port(struct uart_port *port, int flags)
{
	port->type = PORT_S3C2440;
}

static int qin2440_verify_port(struct uart_port *port,
							   struct serial_struct *serial)
{
	// nothing to do
	return 0;
}

static unsigned int qin2440_get_mctrl(struct uart_port *port)
{
	// nothing to do
	return 0;
}

static void qin2440_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	// nothing to do
}

static void qin2440_enable_ms(struct uart_port *port)
{
	// nothing to do
}

static const struct uart_ops qin2440_uart_ops = {
	.tx_empty		= qin2440_tx_empty,
	.get_mctrl		= qin2440_get_mctrl,
	.set_mctrl		= qin2440_set_mctrl,
	.start_tx		= qin2440_start_tx,
	.stop_tx		= qin2440_stop_tx,
	.stop_rx		= qin2440_stop_rx,
	.enable_ms		= qin2440_enable_ms,
	.break_ctl		= qin2440_break_ctl,
	.startup		= qin2440_startup,
	.shutdown		= qin2440_shutdown,
	.set_termios	= qin2440_set_termios,
	.type			= qin2440_type,
	.request_port	= qin2440_request_port,
	.release_port	= qin2440_release_port,
	.config_port	= qin2440_config_port,
	.verify_port	= qin2440_verify_port,
};

#if defined(CONFIG_SERIAL_QIN2440_CONSOLE)

static void qin2440_console_putc(struct uart_port *port, int c)
{
	if(UART_READ_REG(__UFCON, l, port) & 1) {    // if it is fifo mode
		// if tx fifo is not full
		while(UART_READ_REG(__UFSTAT, l, port) & (1 << 14)) {
			barrier();
		}
	} else {
		while(!(UART_READ_REG(__UTRSTAT, l, port) & (1 << 2))) {
			barrier();
		}
	}

	__raw_writeb(c, port->membase);
}

static void qin2440_console_write(struct console *co, const char *s, unsigned int count)
{
	uart_console_write(&qin2440_ports[0].port, s, count, qin2440_console_putc);
}

static int __init qin2440_console_setup(struct console *co, char *options)
{
	struct uart_port *port;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	co->index = 0;
	port = &qin2440_ports[co->index].port;

	if(options) {
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	}

	return uart_set_options(port, co, baud, parity, bits, flow);
}

static struct uart_driver qin2440_driver;

static struct console qin2440_console = {
	.name	= "ttySAC",
	.write	= qin2440_console_write,
	.device	= uart_console_device,
	.setup	= qin2440_console_setup,
	.flags	= CON_PRINTBUFFER,
	.index	= 0,
	.data	= &qin2440_driver,
};

static int __init qin2440_console_init(void)
{
	register_console(&qin2440_console);

	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////
	return 0;
}

console_initcall(qin2440_console_init);

#endif

/*
 *	Define the qin2440 UART driver structure.
 */
static struct uart_driver qin2440_driver = {
	.owner			= THIS_MODULE,
	.driver_name	= "ttySAC",
	.dev_name		= "qin2440_uart",  // this name will lay in "/dev"
	.major			= 204,
	.minor			= 64,
	.nr				= QIN2440_TOTAL_PORTS,
#if defined(CONFIG_SERIAL_QIN2440_CONSOLE)
	.cons			= &qin2440_console,
#endif
};

static int qin2440_probe(struct platform_device *pdev)
{
	int i = pdev->id;

	////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	qin2440_ports[i].port.dev = &pdev->dev;
	uart_add_one_port(&qin2440_driver,  &qin2440_ports[i].port);
	platform_set_drvdata(pdev, &qin2440_ports[i]);

	return 0;
}

static int qin2440_remove(struct platform_device *pdev)
{
	struct qin2440_uart *parent_port = platform_get_drvdata(pdev);

	uart_remove_one_port(&qin2440_driver, &parent_port->port);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver qin2440_platform_driver = {
	.probe		= qin2440_probe,
	.remove		= qin2440_remove,
	.driver		= {
		.name	= "qin2440_uart",
		.owner	= THIS_MODULE,
	},
};

static int __init qin2440_init(void)
{
	int rc;

	while(!((*(volatile unsigned int *)__UTRSTAT0) & (1 << 2)));
	*(volatile unsigned char *)__UTXH0 = 's';

	peripheral_clock_enable(CLKSRC_UART0);
	peripheral_clock_enable(CLKSRC_UART1);
	peripheral_clock_enable(CLKSRC_UART2);

	rc = uart_register_driver(&qin2440_driver);
	if(rc) {
		return rc;
	}

	rc = platform_driver_register(&qin2440_platform_driver);
	if(rc) {
		return rc;
	}

	return 0;
}

static void __exit qin2440_exit(void)
{
	peripheral_clock_disable(CLKSRC_UART0);
	peripheral_clock_disable(CLKSRC_UART1);
	peripheral_clock_disable(CLKSRC_UART2);

	platform_driver_unregister(&qin2440_platform_driver);
	uart_unregister_driver(&qin2440_driver);
}

module_init(qin2440_init);
module_exit(qin2440_exit);
MODULE_LICENSE("GPL");
