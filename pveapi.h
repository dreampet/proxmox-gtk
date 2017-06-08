#include <string>
#include <list>

/*
  Curlite is developed as a lightweight wrapper over cURL
  you can get curlite from: https://github.com/grynko/curlite
*/
#include "curlite/curlite.hpp"
/*
  Jzon is a JSON parser for C++ with focus on a nice and easy to use interface.
  you can get Jzon from: https://github.com/Zguy/Jzon
*/
#include "Jzon/Jzon.h"

#ifndef	__PVEAPI__
#define	__PVEAPI__

/*
Proxmox VE support a REST like API. It choose JSON as primary data format, and the whole API is formally defined using JSON Schema.
You can explore the API documentation at http://pve.proxmox.com/pve-docs/api-viewer/index.html
*/
#define	PVEAPI_GET_DOMAINS	"/api2/json/access/domains"
#define	PVEAPI_GET_TICKET	"/api2/json/access/ticket"
#define	PVEAPI_SET_PASSWORD	"/api2/json/access/password"
#define	PVEAPI_GET_NODES	"/api2/json/nodes"
#define	PVEAPI_GET_VMS		"/api2/json/nodes/{node}/qemu"
#define	PVEAPI_GET_STATUS	"/api2/json/nodes/{node}/qemu/{vmid}/status/current"
#define	PVEAPI_VM_START		"/api2/json/nodes/{node}/qemu/{vmid}/status/start"
#define	PVEAPI_VM_STOP		"/api2/json/nodes/{node}/qemu/{vmid}/status/stop"
#define	PVEAPI_VM_SHUTDOWN	"/api2/json/nodes/{node}/qemu/{vmid}/status/shutdown"
#define	PVEAPI_VM_RESET		"/api2/json/nodes/{node}/qemu/{vmid}/status/reset"
#define	PVEAPI_VM_CONSOLE	"/api2/json/nodes/{node}/qemu/{vmid}/spiceproxy"

enum PVEAPI_VM_STATUS {
	VM_RUNNING,
	VM_STOPPED,
        VM_UNKNOWN,
        VM_INVALID
};

typedef Jzon::Node PVE_NODE;
typedef Jzon::Node PVE_VM;
typedef Jzon::Node PVE_LOGIN;

enum PVEAPI_METHOD {
	METHOD_GET,
	METHOD_POST,
	METHOD_PUT,
	METHOD_AUTO
};

class PVEAPI
{
  public:
    PVEAPI ();
    ~PVEAPI();

  private:
    curlite::Easy	*curl;
    curlite::List	*headers;

  public:
    bool Authentication (PVE_LOGIN *login);

    PVEAPI_VM_STATUS Status(PVE_VM *vm);
    std::list<PVE_NODE *> Nodes();
    std::list<PVE_VM *> Vms( PVE_NODE *node);
    void Start( PVE_VM *vm);
    void Stop( PVE_VM *vm);
    void Shutdown( PVE_VM *vm, int timeout = 5);
    void Reset( PVE_VM *vm );
    void Console( PVE_VM *vm );
    void SetProxmox( std::string proxmox );

  private:
    static bool compare_node(const PVE_NODE * first, const PVE_NODE * second);
    static bool compare_vm(const PVE_VM * first, const PVE_VM * second);

    std::string Format(std::string path, std::string name = "", int vmid = 0);
    Jzon::Node Request(const std::string path, const std::string params = "",
bool withtoken = false, PVEAPI_METHOD method = METHOD_AUTO);

  private:
    std::string	m_ticket;
    std::string	m_csrf;
    std::string m_proxmox;
};

#endif	// __PVEAPI__
