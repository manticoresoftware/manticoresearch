PACKAGE_NAME="manticore"
DEB_FALLBACK_VERSIONED_PACKAGE_NAMES="manticore manticore-server manticore-server-core manticore-tools manticore-dev manticore-common"
RPM_FALLBACK_VERSIONED_PACKAGE_NAMES="manticore manticore-server manticore-server-core manticore-tools manticore-devel manticore-common"
SERVICE_NAME="manticore"
BREW_PACKAGE_NAME="manticoresoftware/tap/manticore"
BREW_SERVICE_NAME="manticore"

DEB_REPO_PACKAGE_NAME="manticore-repo"
RPM_REPO_PACKAGE_NAME="manticore-repo"
RPM_DEV_REPO_PACKAGE_NAME="manticore-dev-repo"

DEB_REPO_PACKAGE_URL="https://repo.manticoresearch.com/manticore-repo.noarch.deb"
DEB_RELEASE_REPO_PACKAGE_URL="$DEB_REPO_PACKAGE_URL"
DEB_DEV_REPO_PACKAGE_URL="https://repo.manticoresearch.com/manticore-dev-repo.noarch.deb"
RPM_REPO_PACKAGE_URL="https://repo.manticoresearch.com/manticore-repo.noarch.rpm"
RPM_RELEASE_REPO_PACKAGE_URL="$RPM_REPO_PACKAGE_URL"
RPM_DEV_REPO_PACKAGE_URL="https://repo.manticoresearch.com/manticore-dev-repo.noarch.rpm"

DEB_REPO_FILE="/etc/apt/sources.list.d/manticoresearch.list"
RPM_RELEASE_REPO_ID="manticore"
RPM_DEV_REPO_ID="manticore-dev"

# Keep in sync with listen directives in ../manticore.conf.in.
DEFAULT_PORTS="9306 9308 9312"

CONF_DIR="/etc/manticoresearch"
DATA_DIR="/var/lib/manticore"
LOG_DIR="/var/log/manticore"

BACKUP_DIR="/var/backups/manticore"
