string(TOLOWER ${PROJECT_NAME} PROJECT_ID)   # Might not be compatible with AppStream
list(APPEND PROJECT_CATEGORIES "Qt") # Freedesktop menu categories
list(APPEND PROJECT_KEYWORDS   "qt;demo;test")
set(PROJECT_AUTHOR_NAME        "weichangk")
set(PROJECT_AUTHOR_EMAIL       "nukgnahciew@gmail.com") # Used also for organization email
set(PROJECT_COPYRIGHT_YEAR     "2023-2024")  # TODO: from git
set(PROJECT_DESCRIPTION        "Qt Demo Test")
set(PROJECT_ORGANIZATION_NAME  "QKit")  # Might be equal to PROJECT_AUTHOR_NAME
set(PROJECT_ORGANIZATION_URL   "${PROJECT_ORGANIZATION_NAME}.github.io")
