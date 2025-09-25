# 🧾 CodBar Generator

CodBar Generator é um aplicativo **Windows Forms** escrito em **VB.NET** no **.NET Framework 4.5** para criar códigos de barras no padrão Code 39.

A interface permite digitar o conteúdo do código, escolher tamanhos pré-definidos e gerar uma imagem pronta para salvar ou copiar para a área de transferência.

---

## 🛠️ Tecnologias Utilizadas

<p align="center">
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/dot-net/dot-net-original.svg" alt=".NET" width="30" height="30"/>
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/vscode/vscode-original.svg" alt="Visual Studio" width="30" height="30"/>
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/windows8/windows8-original.svg" alt="Windows" width="30" height="30"/>
</p>

- **VB.NET** – linguagem principal
- **.NET Framework 4.5** – plataforma alvo
- **Windows Forms** – interface gráfica e `NotifyIcon`
- **Visual Studio** – ambiente de desenvolvimento

---

## 📂 Estrutura do Projeto

- `CodBarGenerator/CodBarGenerator.sln` – solução do Visual Studio 2013/2015
- `CodBarGenerator/CodBarGenerator.vbproj` – projeto principal Windows Forms
- `CodBarGenerator/BR.COM.CODEBARGENERATOR.SCREEN/Main.vb` – lógica da tela e eventos do formulário
- `CodBarGenerator/BR.COM.CODEBARGENERATOR.CLASS/CodeBar.vb` – geração de imagens Code 39 utilizando fonte privada
- `CodBarGenerator/bin/Debug/IDAutomationHC39M.ttf` – fonte necessária para renderizar o código de barras

---

## ✅ Pré-requisitos

- Windows com **.NET Framework 4.5+** instalado
- **Visual Studio 2013** (ou versão mais recente compatível com .NET Framework 4.5)
- Permissões para instalar fontes e executar aplicativos Windows Forms

---

## ⚙️ Configuração

1. **Fonte Code 39**
   - A classe `CodeBar` carrega `IDAutomationHC39M.ttf` a partir da pasta da aplicação (`My.Application.Info.DirectoryPath`).
   - Garanta que o arquivo `.ttf` esteja na mesma pasta do executável ou distribua a fonte com o instalador.

2. **Interface e notificações**
   - O formulário principal (`Main`) utiliza `NotifyIcon` para mostrar balões informativos durante a geração/salvamento.
   - O menu de contexto da imagem permite **Salvar** e **Copiar** rapidamente o código gerado.

3. **Idioma da interface**
   - Os textos estão em inglês, mas podem ser alterados diretamente no `Main.Designer.vb` conforme necessário.

---

## 🛠️ Compilação

1. Abra `CodBarGenerator.sln` no **Visual Studio**.
2. Selecione a configuração desejada (`Debug` ou `Release`).
3. Compile o projeto `CodBarGenerator` (`Build > Build CodBarGenerator`).
4. Os binários e a fonte serão copiados para `CodBarGenerator\CodBarGenerator\bin\<Configuration>`.

---

## ▶️ Execução

1. Execute o aplicativo pelo Visual Studio ou abra `CodBarGenerator.exe` na pasta de saída.
2. Digite o texto desejado no campo **Text CodeBar** (o input é convertido automaticamente para maiúsculas).
3. Escolha um tamanho na lista (`260x85`, `300x85`, `400x85`, `500x85`).
4. Clique em **Generate** para criar a imagem do código de barras.
5. Use os botões/menus para **Resetar**, **Salvar** (`*.png`) ou **Copiar** a imagem para a área de transferência.
6. Minimize para a bandeja do sistema para acessar as ações pelo `NotifyIcon`.

---

## 🔎 Funcionamento

- `CodeBar.GenerateBarCode(width, height)` cria um `Bitmap` com fundo branco e desenha o texto encapsulado por `*` utilizando a fonte Code 39.
- A tela ajusta automaticamente o tamanho da janela e centraliza o formulário conforme a largura da imagem gerada.
- Balões informativos indicam quando a geração ou a gravação do arquivo são concluídas.
- O comando **Reset** limpa o texto, remove a imagem e restaura o tamanho original da janela.

---

## 📌 Observações

- As exceções são tratadas exibindo `MsgBox` com mensagens amigáveis ao usuário.
- O aplicativo encerra chamando `SystemExit`, que fecha o formulário e finaliza o processo.
- Considere atualizar ícones, textos e estilos para adequá-los à identidade do seu projeto.

---

## 📄 Licença

Este projeto está licenciado sob a [MIT License](LICENSE).
