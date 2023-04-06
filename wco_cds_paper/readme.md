
Important:
result limit = 1000 is by default written when the queries are transformed to MDB format.
timeout = 600 is hardcoded in `benchmark_mdb.py`
Building the code
=================
Run ./build.sh

Creating test input file
========================
Edit `create_input_file.sh` to copy input dataset to ./input.txt.

Run ./create_input_file.sh

(Re) Creating MillDB DBs
========================
Edit `recreate_db.sh` as follows:

rm -rf [GRAPH] && ./../build/Release/bin/create_db output.txt [GRAPH]

For example:
1. Using wikidata-filtered-enumerated (1.7 GB)
rm -rf ../tests/dbs/wikipedia_db && ./../build/Release/bin/create_db output.txt ../tests/dbs/wikipedia_db
or
2. Using wikidata-full (19 GB)
rm -rf ../tests/dbs/wikipedia_db_full && ./../build/Release/bin/create_db output.txt ../tests/dbs/wikipedia_db_full

Run ./recreate_db.sh

Running the benchmark
======================

(PENDING EXPLAIN HOW TO TRANSFORM QUERIES FROM RING TO MILLDB FORMAT)

For development use benchmark_local.sh

For actual benchmark run:
./benchmark_server_[filtered|full].*.sh

Optional:

ramdisk parameters
sudo mkdir /mnt/tmp
sudo mount -t tmpfs -o size=15g tmpfs /mnt/tmp
