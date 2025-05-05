#!/bin/bash

# Script to install prerequisites, dependencies for pre-commit hooks, and set Git username

echo "===== Installing Prerequisites ====="

# Update package lists
echo "Updating package lists..."
sudo apt-get update

# Install pipx
echo "Installing pipx..."
sudo apt install pipx -y

# Install Python virtual environment support
echo "Installing Python venv support..."
sudo apt install python3.8-venv -y

# Install pre-commit using pipx
echo "Installing pre-commit..."
pipx install pre-commit

# Ensure pipx binaries are in PATH
echo "Ensuring pipx is in PATH..."
pipx ensurepath

# Set up pre-commit for the current repository
echo "Setting up pre-commit in the repository..."
# Export the new PATH to the current session to ensure pre-commit is found
export PATH="$PATH:$HOME/.local/bin"

# Initialize pre-commit if in a git repository
if [ -d ".git" ]; then
    # Install pre-commit hooks in the current repository
    pre-commit install
    echo "Pre-commit hooks installed successfully!"
else
    echo "Not in a git repository - skipping pre-commit installation."
fi

# Install dependencies for pre-commit hooks
echo "===== Installing Pre-commit Hook Dependencies ====="

# Install C/C++ related tools
echo "Installing C/C++ analysis tools..."
sudo apt install -y flawfinder uncrustify cppcheck clang-tidy bear

# Install cpplint
echo "Installing cpplint..."
pipx install cpplint

echo "Prerequisites installation complete!"
echo "====================================="

# Check if the current directory is a Git repository
if [ ! -d ".git" ]; then
    echo "Warning: Current directory is not a Git repository."
    echo "Git username will not be set."
    echo "Please run this script from the root of a Git repository to set the username."
    exit 1
fi

# Prompt user for their name
echo "Please enter your name for Git commits:"
read git_username

# Validate input is not empty
if [ -z "$git_username" ]; then
    echo "Error: Name cannot be empty."
    exit 1
fi

# Set Git username for the current repository
git config user.name "$git_username"

# Confirm the change
echo "Git username for this repository has been set to: $git_username"
echo "This change only affects the current repository."

# Optionally show the current Git configuration
echo -e "\nCurrent Git configuration for this repository:"
git config --local --get user.name

# Source the current shell configuration to reload PATH
echo -e "\nReloading shell environment..."
if [ -f "$HOME/.bashrc" ]; then
    source "$HOME/.bashrc"
elif [ -f "$HOME/.zshrc" ]; then
    source "$HOME/.zshrc"
fi

# Export the new PATH to the current session
export PATH="$PATH:$HOME/.local/bin"

echo -e "\nSetup complete! Environment has been reloaded."
echo "If you encounter any 'command not found' errors, you may need to manually restart your terminal."
