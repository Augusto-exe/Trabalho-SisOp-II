Trabalho SisOp II

## Requisitos

- g++
- make

## Como rodar

Na pasta raíz do projeto:

Compilar:

`make`

Rodar servidor:

`./servidorTCP`

Rodar clientes:

`./clienteTCP [username] [endereco] 4000`

Ex:

`./clienteTCP @weverton localhost 4000`

## Utilização do Client

Comandos:

Para seguir um usuário:
`FOLLOW [username]`

Para enviar uma mensagem de até 128 caracteres aos seus seguidores:
`SEND [message]`


## Bugs conhecidos. Marcados com X foram resolvidos.

[x] Às vezes as mensagens nao chegam corretamente pra todos. Um sempre recebe corretamente, porem as vezes o outro recebe a mensagem com conteudo errado (um caracter estranho e mais nada).
Se o tamanho do tuite passar de 15 caracteres, uma  passa a receber incorretamente. ex: send 1234567890123456

[X] Apenas a ultima mensagem tuitada pelo usuario eh entregue quando o seu seguidor se conecta.

[X] Segundo usuario recebe mensagens duplicadas quando loga, terceiro recebe triplicado (n-esimo recebe n)

[X] Parar client quando servidor desconectar

