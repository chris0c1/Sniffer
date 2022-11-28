#include "fct.h"
#define SIZE_ETHERNET 14



struct cmd_options parse_cmd(int argc, char **argv)
{
    printf("argc: %d , argv[0] : %s\n",argc,argv[0]);
    struct cmd_options c_o;
    int i = 0;
    int value_letter = 0;
    pcap_if_t *alldevsp = NULL;
    pcap_if_t *device;
    int return_interface;
    (void) return_interface;
    while(argv[1][i] != '\0')
        i++;
    i--;
    c_o.cmd = argv[1][i];
    snprintf(c_o.options,SIZE_OPTION,"%s",argv[2]);
    c_o.options[strnlen(argv[1],SIZE_OPTION)] = '\0';//possiblement inutile

    switch (c_o.cmd)
    {
    case 'i': //interface online
        //check de l'interface 
        printf("find all devs : \n");
        return_interface = pcap_findalldevs(&alldevsp,c_o.options);
        printf("return interface : %d\n",return_interface);
        //(void) alldevsp;
        device = alldevsp;
        //free(alldevsp);
        if(device == NULL) 
        {
            fprintf(stderr,"Erreur sur le nom de l'interface \n");
            exit(1);
        }
        else 
        {
            // traitement interface 
            analyse_online(device);
        }
        free(device);
        break;
    case 'o': //interface offline
        //check du fichier + lancement de l'analyse 
        analyse_offline(c_o.options);
        break;
    case 'f': //filtre BPF (optionnel)
        //check du filtre 
        break;
    case 'v': 
        value_letter = (int) c_o.options[0];
        if((value_letter < 49) || (value_letter > 51))
        {
            fprintf(stderr,"Verbose -> [1|2|3] \n");
            exit(1);
        }
        break;
    
    default:
        //error 
        fprintf(stderr,"Commande possible : -[f|i|o|v]");
        exit(1);
        break;
    }

    

    return c_o; //options et commandes valides 
}


void analyse_offline(char *file)
{
    /*char buf[SIZE_OPTION];
    int fd = open(file,O_RDONLY);
    if(fd == -1) 
    {
        fprintf(stderr,"Erreur ouverture du fichier \n");
        exit(1);
    }
    int read_ = 0;
    int total_size = 0;
    while(read_ = read(fd,buf,SIZE_OPTION) > 0)
    {
        total_size += read;
        //on lit le fichier , //il faut placer le tout dans un plus grand buffer 
    }
    char *full_trame;
    full_trame  = malloc(sizeof(char)*(total_size+1));*/

    /* Decryptage trame */

    //free(full_trame);
    pcap_t *open_file = pcap_open_offline(file,NULL);



    pcap_close(open_file);
}

void analyse_online(pcap_if_t *inter)
{
    (void) inter;

    pcap_t *handle;
    int interface;
    (void) interface;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fp;//optionnel 
    char filtre_exp[] = "";//23 pour password, 21,  ou "" pour pas de filtre	
    bpf_u_int32 masque; // masque
    bpf_u_int32 rs; //reseau 
    struct pcap_pkthdr en_tete; //en-tête général de la trame
    (void) en_tete;

	// recherche automatique de l'interface si mess->"|" en entrée/
    pcap_if_t *alldevsp = NULL;
    pcap_if_t *device;
    (void) device;
	interface = pcap_findalldevs(&alldevsp,errbuf);
    device = alldevsp;
	if(device == NULL) 
    {
        fprintf(stderr,"Erreur pas de dev trouver \n");
        exit(1);
    }
    device = alldevsp;
    // recherche automatique de l'interface si mess->"|" en entrée//
	if(pcap_lookupnet(device->name, &rs, &masque, errbuf) == -1) 
    {
        fprintf(stderr,"Erreur lookupnet : %s\n",errbuf);
        exit(1);
    }
	// Activation du mode promisq //
    // Avec dev precement init //
	handle = pcap_open_live(device->name, BUFSIZ, 1, 1000, errbuf); 
    // 1 pour activer promisq et 1000 de timeout (a changer car 
    // on est en boucle jusqu'à ^C)
	if(handle == NULL) 
    {
        fprintf(stderr,"Erreur dev non accessible : %s\n",errbuf);
        exit(1);
        //erreur 
    }
	// si filtre activer 
	if(pcap_compile(handle, &fp, filtre_exp, 0, rs) == -1)
    {
        fprintf(stderr,"Erreur filtre: %s\n",errbuf);
        exit(1);
        //erreur
    }
    // si filtre activer 
	if(pcap_setfilter(handle, &fp) == -1)
    {
        fprintf(stderr,"Erreur filtre 2 : %s\n",errbuf);
        exit(1);
        //erreur 
    }

   // int n = 20;
	pcap_loop(handle,-1,got_packet,NULL);//search n packet on handle
    pcap_close(handle);
    return;
}

void print_addr(struct in_addr ip_addr, int src_or_dst) //0 for src , 1 for dst
{
    char buffer[INET_ADDRSTRLEN];
    inet_ntop( AF_INET, &ip_addr, buffer, sizeof( buffer ));
    char *message = (!src_or_dst) ? "source": "destination";
    printf( "adresse %s :%s\n", message,buffer );  
}

void print_ip_header(const struct ip * ip)
{
    printf("**IP HEADER**\n");
    printf("Version : %u\n",ip->ip_v);
    printf("IHL : %u\n",ip->ip_hl);

    printf("ip tos : %d\n",ip->ip_tos);
    printf("ip taille : %u\n",ip->ip_len);
    printf("Id : %u\n",ip->ip_id);
    printf("Offset : %u\n",ip->ip_off);
    printf("time to live : %u\n",ip->ip_ttl);
    printf("prot : %u\n",ip->ip_p);//good 
    printf("Checksum : %u\n",ip->ip_sum);
    print_addr((ip->ip_src),0);
    print_addr((ip->ip_dst),1); 
}

void print_udp_header(const struct udphdr * udp)
{
    printf("**UDP HEADER**\n");
    printf("Port Source : %u\n",udp->source);
    printf("Port Destination : %u\n",udp->dest);
    printf("Taille : %u\n",udp->uh_ulen);
    printf("Checksum : %u\n",udp->uh_sum);
}

void print_tcp_header(const struct tcphdr *tcp)
{
    printf("**TCP HEADER**\n");
    printf("Port Source : %u\n",tcp->th_sport);
    printf("Port Destination : %u\n",tcp->th_dport);
    printf("Sequence number : %u\n",tcp->th_seq);
    printf("Acknowledgment number : %u\n",tcp->th_ack);
    printf("Offset : %u\n",tcp->th_off);
    printf("Window : %u\n",tcp->th_win);
    printf("Checkum : %u\n",tcp->th_sum);
    printf("Urgent pointer : %u\n",tcp->th_urp);
    printf("Res1 : %u\n",tcp->res1);
    printf("Off : %u\n",tcp->doff);
    printf("Fin : %u\n",tcp->fin);
    printf("Syn : %u\n",tcp->syn);
    printf("Rst :%u\n",tcp->rst);
    printf("Psh :%u\n",tcp->psh);
    printf("Ack :%u\n",tcp->ack);
    printf("Urg :%u\n",tcp->urg);
    printf("Res2 :%u\n",tcp->res2);
}

void print_mac_adr(unsigned long long mac_adr, int src_or_dst)
{
    (void) src_or_dst;
    (void) mac_adr;

}

void print_arp_header(const struct ether_arp *arp)
{
    printf("**ARP HEADER**\n");
    printf("Hardware type : %u\n",(arp->ea_hdr).ar_hrd);
    printf("Protocole : %u\n",(arp->ea_hdr).ar_pro);
    printf("Type d'adresse physique %u\n",(arp->ea_hdr).ar_hln);
    printf("Taille du type de protocole : %u\n",(arp->ea_hdr).ar_pln);
    printf("Operation : %u\n",(arp->ea_hdr).ar_op);
    printf("Adresse Mac source : %s\n",ether_ntoa((struct ether_addr*)arp->arp_sha));
    (void) arp;
}



void got_packet(u_char *args, const struct pcap_pkthdr *header,
    const u_char *paquet)
{
    //(void) paquet;
    (void) args;
    (void) header;
    const struct ether_header *ethernet;
    const struct ip *ip;
    const struct udphdr *udp; /* The UDP header */
    //const struct ether_arp *arp;

    unsigned int size_ip;
    ethernet = (struct ether_header *)(paquet);
    
    (void) ethernet;
    ip = (struct ip*)(paquet + SIZE_ETHERNET);
    size_ip = (ip->ip_hl)*4; // après des recherches j'ai vu que l'on doit faire HEad length*4
    if (size_ip < 20) {
        fprintf(stderr,"Valeur header length incorrect : %u\n", size_ip);
        exit(1);
    }
    if((ip->ip_p == 17)) {
            //print_ip_header(ip);
            udp = (struct udphdr*)(paquet+SIZE_ETHERNET+size_ip);

            //print_udp_header(udp);
            (void) udp;
            if((udp->source == 67) && (udp->dest == 68)) {
                print_udp_header(udp);
                printf("boot p\n");
            }
        

        /*arp = (struct ether_arp*)(paquet + SIZE_ETHERNET);
        print_arp_header(arp);*/
    }
	return;
}