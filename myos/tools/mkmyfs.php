#!/usr/bin/env php
<?php

 $VERSION               = 1;
 $FILES_MAX             = 32;
 $FLAG_UNUSED           = 0;
 $FLAG_USING            = 1;
 $SIGNATURE1            = 0x77;
 $SIGNATURE2            = "myosFS";
 $HEADER_SIZE           = 28;
 $METADATA_ENTRY_SIZE   = 40;


  //引数チェック

  if($argc < 2){

    print "usage: mkmyfs.php [outfile] file1 file2 ...";
    exit(1);
  }


  //下準備
  $data = '';
  $i    = 0;
  $addr = 0;
  $outfile = $argv[1];


  foreach($argv as $arg){

    $i++;

    if($i < 3)
      continue;

    $filelist[$i-3] = $arg;
  }



  //ヘッダ
  $data .= pack('Ca7VVVVV', $SIGNATURE1,  $SIGNATURE2, $VERSION,
                            $HEADER_SIZE, $HEADER_SIZE,
                            $FILES_MAX*$METADATA_ENTRY_SIZE, $FILES_MAX);


  //メタデータ
  for($i=0; $i < $FILES_MAX; $i++){

    if(isset($filelist[$i])){

      $data .= pack('Ca31VV', $FLAG_USING, basename($filelist[$i]),
                              $addr,       filesize($filelist[$i]));
      $addr += filesize($filelist[$i]);

    }else{

      $data .= pack('Ca31VV', $FLAG_UNUSED, '', 0, 0);
    }
  }


  //データ
  foreach($filelist as $file){

    $data .= file_get_contents($file);

  }


  //ファイルに書き込む
  file_put_contents($outfile, $data);

