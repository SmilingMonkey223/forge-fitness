#!/bin/bash

# FORGE - GitHub Deployment Script
# This script helps you push FORGE to a new GitHub repository

set -e

echo "🚀 FORGE - GitHub Deployment"
echo "=============================="
echo ""

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check if git is configured
if ! git config user.name > /dev/null; then
    echo -e "${RED}Error: Git user.name not configured${NC}"
    echo "Run: git config --global user.name 'Your Name'"
    exit 1
fi

if ! git config user.email > /dev/null; then
    echo -e "${RED}Error: Git user.email not configured${NC}"
    echo "Run: git config --global user.email 'your@email.com'"
    exit 1
fi

echo -e "${GREEN}✓ Git is configured${NC}"
echo "  User: $(git config user.name) <$(git config user.email)>"
echo ""

# Check current status
echo "📊 Current Repository Status:"
echo "  Branch: $(git branch --show-current)"
echo "  Commits: $(git rev-list --count HEAD)"
echo "  Files tracked: $(git ls-files | wc -l)"
echo ""

# Ask for repository URL
echo -e "${YELLOW}Step 1: Create a new repository on GitHub${NC}"
echo "  1. Go to https://github.com/new"
echo "  2. Create a new repository (e.g., 'forge-fitness')"
echo "  3. DO NOT initialize with README, .gitignore, or license"
echo "  4. Copy the repository URL"
echo ""
read -p "Enter the GitHub repository URL (e.g., https://github.com/username/forge-fitness.git): " REPO_URL

if [ -z "$REPO_URL" ]; then
    echo -e "${RED}Error: Repository URL cannot be empty${NC}"
    exit 1
fi

# Remove existing remote if it exists
if git remote get-url origin > /dev/null 2>&1; then
    echo -e "${YELLOW}Removing existing 'origin' remote...${NC}"
    git remote remove origin
fi

# Add new remote
echo -e "${GREEN}Adding remote 'origin'...${NC}"
git remote add origin "$REPO_URL"

# Verify remote
echo "Remote added: $(git remote get-url origin)"
echo ""

# Ask for confirmation
echo -e "${YELLOW}Step 2: Push to GitHub${NC}"
echo "This will push all commits to the main branch."
read -p "Continue? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Aborted."
    exit 1
fi

# Push to GitHub
echo -e "${GREEN}Pushing to GitHub...${NC}"
git push -u origin main

echo ""
echo -e "${GREEN}✓ Successfully pushed to GitHub!${NC}"
echo ""

# Show next steps
echo -e "${YELLOW}🎯 Next Steps:${NC}"
echo ""
echo "1. Set up branch protection:"
echo "   → Go to: https://github.com/$(echo $REPO_URL | sed 's|https://github.com/||' | sed 's|.git||')/settings/branches"
echo "   → Add rule for 'main' branch"
echo "   → Enable: Require PR before merging"
echo "   → Enable: Require status checks (CI must pass)"
echo ""
echo "2. Enable CodeRabbit (optional):"
echo "   → Go to: https://coderabbit.ai"
echo "   → Sign in with GitHub"
echo "   → Install on your repository"
echo ""
echo "3. Create development branch:"
echo "   → git checkout -b develop"
echo "   → git push -u origin develop"
echo ""
echo "4. Start developing with PRs:"
echo "   → git checkout -b feat/your-feature"
echo "   → (make changes)"
echo "   → git push -u origin feat/your-feature"
echo "   → Create PR on GitHub"
echo ""
echo "5. Read the documentation:"
echo "   → README.md - Project overview"
echo "   → QUICKSTART.md - Getting started"
echo "   → CLAUDE.md - Development workflow"
echo "   → PROJECT_STATUS.md - What's next"
echo ""
echo -e "${GREEN}Your repository is live at:${NC}"
echo "  https://github.com/$(echo $REPO_URL | sed 's|https://github.com/||' | sed 's|.git||')"
echo ""
echo "Happy coding! 🏋️"
