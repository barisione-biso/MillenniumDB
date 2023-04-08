sudo sync; sudo echo 3 > /proc/sys/vm/drop_caches
python3 benchmark_mdb.py '/mnt/tmp/MillenniumDB-Dev' 'tests/dbs/wikipedia_db_full' 'output_queries_wikidata_full_no_limit.txt' 'benchmark_results_full_ramdisk.tsv'
