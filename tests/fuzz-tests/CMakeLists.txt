project(fuzztesting)

function(trimtail VAR RESULT)
    STRING(REGEX MATCH "[^/]+$" trimtail_result ${${VAR}})
    set(${RESULT} ${trimtail_result} PARENT_SCOPE)
    STRING(REGEX REPLACE "^(.*)/[^/]+$" "\\1" trimtail_remainder ${${VAR}})
    set(${VAR} ${trimtail_remainder} PARENT_SCOPE)
endfunction(trimtail)

add_custom_target(fuzztest)
file(GLOB_RECURSE tfiles RELATIVE ${PROJECT_SOURCE_DIR} *.fuzz)
foreach (tfile IN LISTS tfiles)
    set(tpath ${tfile})
    trimtail(tpath tname)
    trimtail(tpath textname)
    trimtail(tpath fname)
    add_custom_command(TARGET fuzztest PRE_BUILD COMMAND ${PROJECT_SOURCE_DIR}/../fuzztest ARGS -l fuzzfont-${fname}-${textname}.log -f ${PROJECT_SOURCE_DIR}/../fonts/${fname}.ttf -t 10 -s 60 --memory=200 --include=required,graphite --exclude==fontdir,opentype,volt,advtypo,post --valgrind --input=${PROJECT_SOURCE_DIR}/${fname}/${textname}/${tname} --logfile=${tname} -- ../comparerenderer/comparerenderer -q -s 12 -n -f {} -t ${PROJECT_SOURCE_DIR}/texts/${tname}.txt)
endforeach(tfile)

