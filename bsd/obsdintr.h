#ifndef _OPENBSDINTR_H_
#define _OPENBSDINTR_H_

#ifdef i386
struct intrhand {
	int	(*ih_fun) __P((void *));
	void	*ih_arg;
	u_long	ih_count;
	struct	intrhand *ih_next;
	int	ih_level;
	int	ih_irq;
	char	*ih_what;
};
#endif

#ifdef pc532
struct iv {
	void (*iv_vec)();
	void *iv_arg;
	int iv_cnt;
	char *iv_use;
};
#endif

#endif /* _OPENBSDINTR_H_ */
