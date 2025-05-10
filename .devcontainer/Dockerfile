# Usa a imagem base oficial do Python + VS Code
FROM mcr.microsoft.com/devcontainers/python:1-3.11

# Instala o Node.js (necessário para o frontend Vue)
RUN apt-get update && \
    apt-get install -y nodejs npm && \
    npm install -g n && \
    n 16.14.0 && \
    apt-get clean

# Instala dependências do sistema para sensores (simulados)
RUN apt-get install -y \
    build-essential \
    python3-dev

# Configura o ambiente Python
RUN pip install --upgrade pip setuptools wheel

# Cria um usuário não-root (boas práticas)
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID
RUN groupadd --gid $USER_GID $USERNAME && \
    useradd --uid $USER_UID --gid $USER_GID -m $USERNAME && \
    echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME && \
    chmod 0440 /etc/sudoers.d/$USERNAME

# Diretório de trabalho
WORKDIR /workspace
