bind pub - !bnc bnccheck


 
set exe "/home/hawk/eggdrop/bnccheck"
set user "anonymous"
set pass "a@b.de"
set ip [list "ftp.debian.de" "ftp.kernel.org" "1.1.1.1"]
set port [list "21" "21" "31000"]
set ssl [list "1" "0" "0"]
set color [list "1" "1" "0"]
set sitetag "TeSt"


proc bnccheck {nick uhost hand chan args} {
global exe
global user
global pass
global ip
global port
global sitetag
global ssl
global color

putquick "PRIVMSG $chan : =14PPX BNC CHECK= Checking [llength $ip] BNC(s) for '$sitetag' please stand by.."

for {set i 0} {$i < [llength $ip]} {incr i} {
foreach  line  [split [ exec $exe [lindex $ip $i] [lindex $port $i] $user $pass [lindex $ssl $i] [lindex $color $i]] \n] {
set count [expr {$i + 1}]
putquick "PRIVMSG $chan :   -> (BNC #$count) [lindex $ip $i]:[lindex $port $i] ($line)"

}
}
}
