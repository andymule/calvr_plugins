SET(LIB_NAME GML)
SET(PLUGIN_HEADERS
    GML.h
)

ADD_LIBRARY(${LIB_NAME}
    "SHARED"
    ${PLUGIN_HEADERS}
    GML.cpp
)

FIND_PACKAGE(Igraph)

INCLUDE_DIRECTORIES(${OSG_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${IGRAPH_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${LIB_NAME} ${OSG_LIBRARIES})
TARGET_LINK_LIBRARIES(${LIB_NAME} ${IGRAPH_LIBRARY})

INSTALL(TARGETS ${LIB_NAME} DESTINATION lib/plugins)

