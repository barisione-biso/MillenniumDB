sudo umount /mnt/tmp
sudo mount -t tmpfs -o size=210g tmpfs /mnt/tmp
cp -R /data/MilleniumDB-Dev /mnt/tmp
cd /mnt/tmp/MillenniumDB-Dev
sudo sync; sudo echo 3 > /proc/sys/vm/drop_caches
python3 benchmark_mdb.py '/mnt/tmp/MillenniumDB-Dev' 'tests/dbs/wikipedia_db_full' 'output_queries_wikidata_full_no_limit.txt' 'benchmark_results_full_ramdisk_no_limit.tsv'
