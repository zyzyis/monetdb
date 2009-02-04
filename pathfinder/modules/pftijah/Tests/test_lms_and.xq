let $opt := <TijahOptions 
                ft-index="thesis" 
                ir-model="LMS" 
                return-all="false"
                debug="0"/>

let $query := "//title[about(.,pathfinder) and about(.,TIJAH)]"

for $n at $r in tijah:queryall($query,$opt)
return <node rank="{$r}">{$n}</node>
