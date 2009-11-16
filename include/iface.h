extern "C" {
#include <errno.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <sys/time.h>
#include <netinet/icmp6.h>
#include <linux/if.h>             /* for IFNAMSIZ */
#include "rpl.h"
}

#define HWADDR_MAX 16

class network_interface {

public:
	int announce_network();
	network_interface();
	network_interface(int fd);
	network_interface(const char *if_name);

        // setup the object, open sockets, etc.
        bool setup(void);

	int errors(void) {
		return error_cnt;
	}
	void set_verbose(int flag) { verbose_flag = flag; }
	void set_verbose(int flag, FILE *out) {
		verbose_flag = flag;
		verbose_file = out;
	}
	int  verboseprint() { return verbose_flag; }

	virtual void receive_packet(struct in6_addr ip6_src,
				    struct in6_addr ip6_dst,
				    const u_char *bytes, const int len);

        void receive_dao(const u_char *dao_bytes, const int dao_len);
        void receive_dio(const u_char *dio_bytes, const int dio_len);

        void send_dio(void);
        void send_raw_dio(unsigned char *icmp_body, unsigned int icmp_len);
        int  build_dio(unsigned char *buff, unsigned int buff_len);

        void set_if_name(const char *ifname);
        void set_rpl_dagid(const char *dagstr);
        void set_rpl_dagrank(const int dagrank) {
                rpl_dagrank = dagrank;
        };
        void set_rpl_sequence(const int sequence) {
                rpl_sequence = sequence;
        };
        void set_rpl_instanceid(const int instanceid) {
                rpl_instanceid = instanceid;
        };
        void update_multicast_time(void) {
		struct timeval tv;

		gettimeofday(&tv, NULL);

		last_multicast_sec = tv.tv_sec;
		last_multicast_usec = tv.tv_usec;
        };

        
        static void main_loop(FILE *verbose);

private:
	int packet_too_short(const char *thing, const int avail, const int needed);
	int                     nd_socket;
	int                     error_cnt;
        bool                    alive;

        int                     get_if_index(void);
        int                     if_index;      /* cached value for above */

        char                    if_name[IFNAMSIZ];
        struct in6_addr         if_addr;
	int			if_prefix_len;

	uint8_t			if_hwaddr[HWADDR_MAX];
	int			if_hwaddr_len;
	int			if_maxmtu;

        /* RiPpLe statistics */
        int                     rpl_grounded;
        int                     rpl_sequence;
        int                     rpl_instanceid;
        int                     rpl_dagrank;
        unsigned char           rpl_dagid[16];

        /* timers */
	time_t			last_multicast_sec;
	suseconds_t		last_multicast_usec;
        

        /* debugging */
	int                     verbose_flag;
	FILE                   *verbose_file;
#define VERBOSE(X) ((X)->verbose_flag && (X)->verbose_file!=NULL)


        unsigned char          *control_msg_hdr;
        unsigned int            control_msg_hdrlen;

        /* read from our network socket and process result */
        void receive(void);

        /* private helper functions */
        void setup_allrouters_membership(void);
        void check_allrouters_membership(void);



        /* maintain list of all interfaces */
        void add_to_list(void);

        class network_interface        *next;
        static class network_interface *all_if;
        static int                      if_count(void);
};

#define ND_OPT_RPL_PRIVATE_DAO 200
#define ND_OPT_RPL_PRIVATE_DIO 201