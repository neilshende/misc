alias dev='echo -n "Welcome123!" | pbcopy && ssh viveks:Welcome123!@10.0.1.75'
alias tst='echo -n "Welcome123!" | pbcopy && ssh viveks:Welcome123!@10.0.1.62'
alias ff='find . -iname'
alias dot='. ~/.bashrc'
alias vi='vim'
alias ec2='ssh -i "~/vivek-key.pem" ec2-user@ec2-3-139-93-220.us-east-2.compute.amazonaws.com'
alias shep='ssh -i ~/sheperd.pem centos@ec2-3-138-69-149.us-east-2.compute.amazonaws.com'
function wssh
{
  cp ~/.ssh/known_hosts ~/.ssh/kh
  sed $1d < ~/.ssh/kh > ~/.ssh/known_hosts
}
function sshnopasswd
{
  ssh-keygen -t rsa
  cat ~/.ssh/id_rsa.pub | ssh $1 'cat >> .ssh/authorized_keys'
}
PATH=$PATH:~/bin
