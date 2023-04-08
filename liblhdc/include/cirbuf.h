#ifndef	_CIRBUF_H_
#define	_CIRBUF_H_

struct cirbuf_s
{
	unsigned int	idx;
	unsigned int	odx;
	unsigned int	s_len;
	unsigned int	r_len;
	unsigned int	max_len;
	unsigned char	*cbuf;
};
typedef  struct cirbuf_s savi_circ_buf;

void cirbuf_init( struct cirbuf_s *pcb, unsigned char *buf, int len);

void cirbuf_reset( struct cirbuf_s *pcb);

int cirbuf_len( struct cirbuf_s *pcb);
int cirbuf_empty_len( struct cirbuf_s *pcb);

int cirbuf_get( struct cirbuf_s *pcb, unsigned char *buf, int len);
int cirbuf_put( struct cirbuf_s *pcb, unsigned char *buf, int len);

int cirbuf_get_no_copy( struct cirbuf_s *pcb, unsigned char **buf, int len);
int cirbuf_put_no_copy( struct cirbuf_s *pcb, unsigned char **buf, int len);

#endif	/* _CIRBUF_H_ */

