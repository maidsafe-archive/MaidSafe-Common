PROJECT(breakpad-client)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

SET(CMAKE_DEBUG_POSTFIX _d)

SET(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(src)

SET(SOURCES)

IF(UNIX)
  ADD_DEFINITIONS(-Wall)
  SET(SOURCES
    src/client/minidump_file_writer.cc
    src/common/convert_UTF.c
    src/common/string_conversion.cc
  )
  IF(APPLE)
    INCLUDE_DIRECTORIES(/Developer/Headers/FlatCarbon)
    FIND_LIBRARY(COCOA_LIBRARY COCOA)
    MARK_AS_ADVANCED(COCOA_LIBRARY)
    SET(EXTRA_LIBS crypto ${COCOA_LIBRARY})
    SET(SOURCES ${SOURCES}
	src/client/mac/handler/breakpad_nlist_64.cc
	src/client/mac/handler/dynamic_images.cc
	src/client/mac/handler/exception_handler.cc
	src/client/mac/handler/minidump_generator.cc
	src/client/mac/handler/protected_memory_allocator.cc
	src/common/mac/dump_syms.mm
#	src/common/mac/dwarf/bytereader.cc
#	src/common/mac/dwarf/dwarf2reader.cc
#	src/common/mac/dwarf/functioninfo.cc
	src/common/mac/file_id.cc
	src/common/mac/macho_id.cc
	src/common/mac/macho_utilities.cc
	src/common/mac/macho_walker.cc
	src/common/mac/string_utilities.cc    
)
  ELSE()
    SET(SOURCES ${SOURCES}
      src/client/linux/handler/exception_handler.cc
      #src/client/linux/handler/linux_thread.cc
      #src/client/linux/handler/minidump_generator.cc
      src/client/linux/crash_generation/crash_generation_client.cc #ex1
      src/client/linux/minidump_writer/minidump_writer.cc #ex2
      src/client/linux/minidump_writer/linux_dumper.cc #ex3
      src/common/linux/dump_symbols.cc
      src/common/linux/file_id.cc
      src/common/linux/guid_creator.cc
      src/common/linux/http_upload.cc
      src/common/md5.c
      src/common/dwarf_cfi_to_module.cc
      src/common/dwarf_cu_to_module.cc
      src/common/dwarf_line_to_module.cc
      src/common/dwarf/bytereader.cc
      src/common/dwarf/dwarf2reader.cc
      src/common/module.cc
      src/common/language.cc
      src/common/linux/elf_symbols_to_module.cc
      src/common/stabs_to_module.cc
    )
  ENDIF()
ELSEIF(WIN32)
  SET(SOURCES
    src/common/windows/guid_string.cc
    src/client/windows/crash_generation/crash_generation_client.cc
    src/client/windows/crash_generation/client_info.cc
    src/client/windows/crash_generation/minidump_generator.cc
    src/client/windows/handler/exception_handler.cc
  )
  #ADD_DEFINITIONS(/Zc:wchar_t-)
  ADD_DEFINITIONS(/D "WIN32" /D "WIN32_LEAN_AND_MEAN" /D "_UNICODE" /D "UNICODE" /EHsc)
  SET(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /Ot /Oy /GL /D \"NDEBUG\" /MT /Gy /Zi")
  IF(CMAKE_CL_64)
    SET(CMAKE_CXX_FLAGS_DEBUG "/Od /Ot /Oy /D \"_DEBUG\" /D \"DEBUG\" /MTd /c /Zi /TP")
  ELSE()
    SET(CMAKE_CXX_FLAGS_DEBUG "/Od /Ot /Oy /D \"_DEBUG\" /D \"DEBUG\" /MTd /c /ZI /TP")
  ENDIF()
ENDIF()

ADD_LIBRARY(breakpad STATIC ${SOURCES})

IF(UNIX AND NOT APPLE)
  SET_TARGET_PROPERTIES(breakpad PROPERTIES COMPILE_FLAGS -fPIC)
ENDIF()

TARGET_LINK_LIBRARIES(breakpad ${EXTRA_LIBS})
