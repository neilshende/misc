PS1="\[\e[32m\]\u\[\e[m\]\[\e[36m\]@\[\e[m\]\[\e[34m\]\W\[\e[m\]\$ "
alias g++='g++ -g -std=c++20'
alias gpp='g++ -g -std=c++20'
alias gcc='g++ -g -std=c++20'
alias cc='g++ -g -std=c++20'
alias ls='ls -G'
alias gs='git status'
alias gd='git diff'
alias gde='git diff origin/master'
alias gc='git commit --amend'
alias gm='git commit -m'
alias gl='git log'
alias gpull='git pull --rebase'
alias gpush='git push'
alias guncommit='git reset --hard HEAD~1'
alias viba='vim ~/.bashrc'
alias dot='. ~/.bashrc'
alias ubuntu='ssh vivek:vivek@192.168.1.219'
export VMTK_ENV=virtualbox
export MAKEOBJDIRPREFIX=~/obj

alias ack='ack -i -u'
function fim
{
   vim $(ff $1 2>/dev/null | head -n 1)
}
function devcp
{
   pushd ~/Downloads
   scp vshende@vshendev-vm $1 ~/Downloads/
}
function restart
{
   sudo ifconfig en0 down
   sudo ifconfig en0 up
}

function wssh
{
   cp ~/.ssh/known_hosts ~/.ssh/kh
   sed $1d < ~/.ssh/kh > ~/.ssh/known_hosts
}
function sshnopasswd
{
# next line needed first time only.
#    ssh-keygen -t rsa
    cat ~/.ssh/id_rsa.pub | ssh $1 'cat >> .ssh/authorized_keys'
}
function dofio
{
    fio --thread --direct=1 --bs=512 --ioengine=libaio --iodepth=1 --ramp_time=10 --runtime=300 --rw=randwrite --time_based --name=mpath3 --filename=$1
}
function connect
{
    while true; do ssh ${1}; sleep 2; done
}
function gack
{
    grep -r "$1" *
}
function ff
{
    find . -name $1
}
function test
{
   echo $#
   echo $*
}
function loop_till_success
{
   while ! `$*` ;do sleep 1; done
}

function loop
{
    while true; do a=`$*`;echo $a; sleep 2; done
}
function looper
{
    if [ $# -ne 3 ] ;then
        echo Usage: looper \"cmd\" loopcnt delay
        return
    fi
    for i in `seq 1 $2`
    do 
        a=`$1`
        echo $i">>" $a
        sleep $3
    done
}
function pi
{
  if [ $# -ne 1 ] ;then
      scale=100
  else
      scale=$1
  fi
  bc << END-OF-INPUT
  scale=$scale
  define myfunc(s,j){
    return (s*4)/(j*(j+1)*(j+2))
  }
  s=1
  j=2
  pi=3
  for (i=1; i<scale; i++) {
    pi += myfunc(s,j)
    j=j+2
    s=-s
  }
print pi, "\n"
END-OF-INPUT
}
