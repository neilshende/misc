syntax on
set shiftwidth=4
set tabstop=4
set expandtab
set autoindent
set smartindent
set nu
set paste
set ffs=unix
set encoding=utf-8
set fileencoding=utf-8
set listchars=tab:>-,trail:.
set list
highlight SpecialKey ctermfg=red
nnoremap <F5> :let _s=@/<Bar>:%s/\s\+$//e<Bar>:let @/=_s<Bar><CR>
map <f12> :set list!\|set number!<cr>
map <F2> :retab <CR>
