#!/bin/sh

echo "rm -f hac_out                      hac_out.dbscan               hac_out.default              hac_out.fast                 hac_out.stringy"
      rm -f hac_out                      hac_out.dbscan               hac_out.default              hac_out.fast                 hac_out.stringy

echo "rm -f hac_out.clusters             hac_out.dbscan.clusters      hac_out.default.clusters     hac_out.fast.clusters        hac_out.stringy.clusters"
      rm -f hac_out.clusters             hac_out.dbscan.clusters      hac_out.default.clusters     hac_out.fast.clusters        hac_out.stringy.clusters

echo "rm -f hac10K_out                   hac10K_out.dbscan            hac10K_out.default           hac10K_out.fast              hac10K_out.stringy"
      rm -f hac10K_out                   hac10K_out.dbscan            hac10K_out.default           hac10K_out.fast              hac10K_out.stringy

echo "rm -f hac10K_out.clusters          hac10K_out.dbscan.clusters   hac10K_out.default.clusters  hac10K_out.fast.clusters     hac10K_out.stringy.clusters"
      rm -f hac10K_out.clusters          hac10K_out.dbscan.clusters   hac10K_out.default.clusters  hac10K_out.fast.clusters     hac10K_out.stringy.clusters

echo "rm -rf __pycache__"
      rm -rf __pycache__

