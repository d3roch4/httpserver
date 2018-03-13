
#include "bitserver.h"

int despachante(SessaoHttp* ctx)
{
    RespHttp* resp = ctx->resp;
    respHttp_enviarStatus(resp, STATUS_HTTP_OK);
    respHttp_enviarCabecalho(resp, "Content-Type", "text/html; charset=utf-8");
    respHttp_enviarCabecalho(resp, "Connection", "close");

    respHttp_enviarParcial(resp, "<ol>");
    char* linhas[] = {"linha1", "linha2", "linha3", "linha4"};
    for (int i = 0; i < 4; i++) {
        respHttp_enviarParcial(resp, "<li>");
        respHttp_enviarParcial(resp, linhas[i]);
        respHttp_enviarParcial(resp, "</li>");
    }

    respHttp_enviarFinal(resp, "</ol>");
    return 0;
}

int main()
{
    bitserver_iniciar("3000", "./", despachante);
    return 0;
}
