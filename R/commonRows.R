data <- read.delim("rc.tsv", header = TRUE, row.names = 1) 
commonrows = c()
 for (rowname in rownames(data)) {
    if (!grepl("Readthrough", rowname)) next;
    otherrow = gsub("Readthrough", "Genecount", rowname)
    if (otherrow %in% rownames(data)) {
       commonrows = append(commonrows, rowname)
    }
 }
data = data[commonrows, ]
