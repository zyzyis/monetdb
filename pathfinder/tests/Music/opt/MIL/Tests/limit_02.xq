(: q2 'SELECT * FROM Assets WHERE TrackNr=1 LIMIT 100' :)
import module namespace music = "http://www.cwi.nl/~boncz/music/opt/" at "http://www.cwi.nl/~boncz/music/opt/music.mil";
music:AssetByTrackNr1("music.xml", 100) 
