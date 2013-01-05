<?php
$nss = array();
$ns_res = array();	  
function select_ns($file, $lang)
{
	global $nss, $ns_res;
	
	include($file);
	if (isset($namespaceNames)) {
		array_push($nss, $lang);
		printf("wiki_ns_%s = {\n", $lang);
		$reg = array();
		foreach ($namespaceNames as $ns => $val) {
			if ($val == '')
				continue;
			if (preg_match('/(.*)\$1(.*)/', $val, $matches)) 
				array_push($reg,
					   array($matches[1], $matches[2],
						 $ns));
			else
				printf("    \"%s\": \"%s\",\n", $val, $ns);
 	        }
		print "}\n";
		if (count($reg)) {
			array_push($ns_res, $lang);
			printf("wiki_ns_re_%s = [\n", $lang);
			foreach ($reg as $val) {
				printf("    (\"%s\", \"%s\", \"%s\"),\n",
				       $val[0], $val[1], $val[2]);
			}
			print "]\n";
		}
	}
}
		

function process_dir($dir)
{
	if (!($dp = opendir ($dir)))
		die ("Can't open data directory.");
	while ($name = readdir ($dp)) {
		if (preg_match('|.*/?Messages([A-Z][a-z_-]+).php|', $name, $matches)) {
			$lang = strtolower($matches[1]);
			#print "$lang\n";
			select_ns($name, $lang);
		}
			
	}
	closedir ($dp);
}

echo "# This file is generated automatically. Do not edit.\n";
echo "# -*- coding: utf-8 -*-\n";
process_dir(".");

print "wiki_ns = {\n";
if (count($nss)) {
	foreach ($nss as $lang) 
		printf("    \"%s\": wiki_ns_%s,\n", $lang, $lang);
}
print "}\n";

print "wiki_ns_re = {\n";
if (count($ns_res)) {
	foreach ($nss as $lang) 
		printf("    \"%s\": wiki_ns_re_%s,\n", $lang, $lang);
}
print "}\n";


?>
