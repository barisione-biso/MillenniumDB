sudo umount /mnt/tmp
sudo mount -t tmpfs -o size=210g tmpfs /mnt/tmp
cp -R /data/MillenniumDB-Dev /mnt/tmp
cd /mnt/tmp/MillenniumDB-Dev