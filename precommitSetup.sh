#!/usr/bin/env sh
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

# Install dependencies for pre-commit hooks
echo "===== Installing Pre-commit Hook Dependencies ====="

# Install C/C++ related tools
echo "Installing C/C++ analysis tools..."
sudo apt install -y flawfinder uncrustify cppcheck clang-tidy

# Install cpplint
echo "Installing cpplint..."
pipx install cpplint
pipx ensurepath

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

echo -e "\nSetup complete! You may need to restart your terminal for all changes to take effect."
