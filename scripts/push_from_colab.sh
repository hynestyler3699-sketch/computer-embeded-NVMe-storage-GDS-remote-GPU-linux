#!/usr/bin/env bash
# push_from_colab.sh
# Helper script to commit and push changes from Google Colab
#
# Usage:
#   export GITHUB_TOKEN="ghp_your_token_here"
#   ./scripts/push_from_colab.sh "Commit message" [files...]
#
# If no files specified, defaults to notebooks/

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Colab Push Helper ===${NC}"

# Check for token
if [ -z "${GITHUB_TOKEN:-}" ]; then
    echo -e "${RED}ERROR: GITHUB_TOKEN environment variable not set${NC}"
    echo ""
    echo "Set your GitHub Personal Access Token:"
    echo "  export GITHUB_TOKEN=\"ghp_your_token_here\""
    echo ""
    echo "To get a token:"
    echo "  1. Go to https://github.com/settings/tokens"
    echo "  2. Generate a new token with 'repo' scope"
    echo "  3. Copy and export it"
    exit 1
fi

# Get commit message
MSG="${1:-Update from Colab}"
shift 2>/dev/null || true

# Get files to commit (default to notebooks/)
if [ $# -gt 0 ]; then
    FILES="$@"
else
    FILES="notebooks/"
fi

echo -e "${YELLOW}Commit message:${NC} $MSG"
echo -e "${YELLOW}Files:${NC} $FILES"

# Check if we're in a git repo
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo -e "${RED}ERROR: Not in a git repository${NC}"
    exit 1
fi

# Get current branch
BRANCH=$(git rev-parse --abbrev-ref HEAD)
echo -e "${YELLOW}Branch:${NC} $BRANCH"

# Stage files
echo ""
echo -e "${GREEN}Staging files...${NC}"
git add $FILES

# Check if there's anything to commit
if git diff --cached --quiet; then
    echo -e "${YELLOW}Nothing to commit - files are unchanged${NC}"
    exit 0
fi

# Show what will be committed
echo ""
echo -e "${GREEN}Changes to be committed:${NC}"
git diff --cached --stat

# Commit
echo ""
echo -e "${GREEN}Committing...${NC}"
git commit -m "$MSG"

# Push using token
echo ""
echo -e "${GREEN}Pushing to origin...${NC}"

REPO_URL=$(git config --get remote.origin.url 2>/dev/null || echo "")

if [ -z "$REPO_URL" ]; then
    # Default repo if no remote set
    REPO_URL="https://github.com/hynestyler3699-sketch/computer-embeded-NVMe-storage-GDS-remote-GPU-linux.git"
    echo -e "${YELLOW}No remote found, using default: $REPO_URL${NC}"
fi

# Convert SSH URL to HTTPS if needed
if [[ "$REPO_URL" == git@github.com:* ]]; then
    REPO_URL=$(echo "$REPO_URL" | sed 's/git@github.com:/https:\/\/github.com\//' | sed 's/\.git$//')
    REPO_URL="${REPO_URL}.git"
fi

# Remove any existing credentials from URL and add token
CLEAN_URL=$(echo "$REPO_URL" | sed 's|https://[^@]*@|https://|')
AUTH_URL=$(echo "$CLEAN_URL" | sed "s|https://|https://${GITHUB_TOKEN}@|")

git push "$AUTH_URL" "$BRANCH"

echo ""
echo -e "${GREEN}=== Push complete! ===${NC}"
echo ""
echo "View your changes at:"
echo "  https://github.com/hynestyler3699-sketch/computer-embeded-NVMe-storage-GDS-remote-GPU-linux/tree/$BRANCH"
