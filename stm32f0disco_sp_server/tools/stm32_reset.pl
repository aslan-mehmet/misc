#!/usr/bin/perl
# NOTE: needs libnet-telnet-perl package. 
use Net::Telnet;
use Cwd 'abs_path';
 
my $numArgs = $#ARGV + 1;
if($numArgs != 1){
    die( "Usage ./stm32_flash.pl [main.bin] \n");
}

my $file = abs_path($ARGV[0]);

my $ip = "127.0.0.1"; 
my $port = 4444;

my $telnet = new Net::Telnet (
    Port   => $port,
    Timeout=> 30,
    Errmode=> 'die',
    Prompt => '/>/');

$telnet->open($ip);

print $telnet->cmd('poll');
print $telnet->cmd('reset halt');
print $telnet->cmd('exit');

print "\n";
