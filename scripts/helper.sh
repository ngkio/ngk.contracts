# Ensures passed in version values are supported.
function check-version-numbers() {
  CHECK_VERSION_MAJOR=$1
  CHECK_VERSION_MINOR=$2

  if [[ $CHECK_VERSION_MAJOR -lt $NGK_MIN_VERSION_MAJOR ]]; then
    exit 1
  fi
  if [[ $CHECK_VERSION_MAJOR -gt $NGK_MAX_VERSION_MAJOR ]]; then
    exit 1
  fi
  if [[ $CHECK_VERSION_MAJOR -eq $NGK_MIN_VERSION_MAJOR ]]; then
    if [[ $CHECK_VERSION_MINOR -lt $NGK_MIN_VERSION_MINOR ]]; then
      exit 1
    fi
  fi
  if [[ $CHECK_VERSION_MAJOR -eq $NGK_MAX_VERSION_MAJOR ]]; then
    if [[ $CHECK_VERSION_MINOR -gt $NGK_MAX_VERSION_MINOR ]]; then
      exit 1
    fi
  fi
  exit 0
}


# Handles choosing which NGK directory to select when the default location is used.
function default-ngk-directories() {
  REGEX='^[0-9]+([.][0-9]+)?$'
  ALL_NGK_SUBDIRS=()
  if [[ -d ${HOME}/ngk ]]; then
    ALL_NGK_SUBDIRS=($(ls ${HOME}/ngk | sort -V))
  fi
  for ITEM in "${ALL_NGK_SUBDIRS[@]}"; do
    if [[ "$ITEM" =~ $REGEX ]]; then
      DIR_MAJOR=$(echo $ITEM | cut -f1 -d '.')
      DIR_MINOR=$(echo $ITEM | cut -f2 -d '.')
      if $(check-version-numbers $DIR_MAJOR $DIR_MINOR); then
        PROMPT_NGK_DIRS+=($ITEM)
      fi
    fi
  done
  for ITEM in "${PROMPT_NGK_DIRS[@]}"; do
    if [[ "$ITEM" =~ $REGEX ]]; then
      NGK_VERSION=$ITEM
    fi
  done
}


# Prompts or sets default behavior for choosing NGK directory.
function ngk-directory-prompt() {
  if [[ -z $NGK_DIR_PROMPT ]]; then
    default-ngk-directories;
    echo 'No NGK location was specified.'
    while true; do
      if [[ $NONINTERACTIVE != true ]]; then
        if [[ -z $NGK_VERSION ]]; then
          echo "No default NGK installations detected..."
          PROCEED=n
        else
          printf "Is NGK installed in the default location: $HOME/ngk/$NGK_VERSION (y/n)" && read -p " " PROCEED
        fi
      fi
      echo ""
      case $PROCEED in
        "" )
          echo "Is NGK installed in the default location?";;
        0 | true | [Yy]* )
          break;;
        1 | false | [Nn]* )
          if [[ $PROMPT_NGK_DIRS ]]; then
            echo "Found these compatible NGK versions in the default location."
            printf "$HOME/ngk/%s\n" "${PROMPT_NGK_DIRS[@]}"
          fi
          printf "Enter the installation location of NGK:" && read -e -p " " NGK_DIR_PROMPT;
          NGK_DIR_PROMPT="${NGK_DIR_PROMPT/#\~/$HOME}"
          break;;
        * )
          echo "Please type 'y' for yes or 'n' for no.";;
      esac
    done
  fi
  export NGK_INSTALL_DIR="${NGK_DIR_PROMPT:-${HOME}/ngk/${NGK_VERSION}}"
}


# Prompts or default behavior for choosing NGK.CDT directory.
function cdt-directory-prompt() {
  if [[ -z $CDT_DIR_PROMPT ]]; then
    echo 'No NGK.CDT location was specified.'
    while true; do
      if [[ $NONINTERACTIVE != true ]]; then
        printf "Is NGK.CDT installed in the default location? /usr/local/ngk.cdt (y/n)" && read -p " " PROCEED
      fi
      echo ""
      case $PROCEED in
        "" )
          echo "Is NGK.CDT installed in the default location?";;
        0 | true | [Yy]* )
          break;;
        1 | false | [Nn]* )
          printf "Enter the installation location of NGK.CDT:" && read -e -p " " CDT_DIR_PROMPT;
          CDT_DIR_PROMPT="${CDT_DIR_PROMPT/#\~/$HOME}"
          break;;
        * )
          echo "Please type 'y' for yes or 'n' for no.";;
      esac
    done
  fi
  export CDT_INSTALL_DIR="${CDT_DIR_PROMPT:-/usr/local/ngk.cdt}"
}


# Ensures NGK is installed and compatible via version listed in tests/CMakeLists.txt.
function nodengk-version-check() {
  INSTALLED_VERSION=$(echo $($NGK_INSTALL_DIR/bin/nodengk --version))
  INSTALLED_VERSION_MAJOR=$(echo $INSTALLED_VERSION | cut -f1 -d '.' | sed 's/v//g')
  INSTALLED_VERSION_MINOR=$(echo $INSTALLED_VERSION | cut -f2 -d '.' | sed 's/v//g')

  if [[ -z $INSTALLED_VERSION_MAJOR || -z $INSTALLED_VERSION_MINOR ]]; then
    echo "Could not determine NGK version. Exiting..."
    exit 1;
  fi

  if $(check-version-numbers $INSTALLED_VERSION_MAJOR $INSTALLED_VERSION_MINOR); then
    if [[ $INSTALLED_VERSION_MAJOR -gt $NGK_SOFT_MAX_MAJOR ]]; then
      echo "Detected NGK version is greater than recommended soft max: $NGK_SOFT_MAX_MAJOR.$NGK_SOFT_MAX_MINOR. Proceed with caution."
    fi
    if [[ $INSTALLED_VERSION_MAJOR -eq $NGK_SOFT_MAX_MAJOR && $INSTALLED_VERSION_MINOR -gt $NGK_SOFT_MAX_MINOR ]]; then
      echo "Detected NGK version is greater than recommended soft max: $NGK_SOFT_MAX_MAJOR.$NGK_SOFT_MAX_MINOR. Proceed with caution."
    fi
  else
    echo "Supported versions are: $NGK_MIN_VERSION_MAJOR.$NGK_MIN_VERSION_MINOR - $NGK_MAX_VERSION_MAJOR.$NGK_MAX_VERSION_MINOR"
    echo "Invalid NGK installation. Exiting..."
    exit 1;
  fi
}
