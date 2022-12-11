head -n 100000 ~/dcc_uchile/git_projects/data/wikidata-filtered-enumerated.dat > input.txt 
./ring_to_millDB_format_DB_converter input.txt 
