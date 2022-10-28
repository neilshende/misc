i = read.delim("amp.tsv", stringsAsFactors=FALSE)
m <- read.delim("~/mart_exportGeneNames.txt", stringsAsFactors=FALSE)
im = merge(i,m, by.x="IAGENE", by.y="Gene.name", all.x=T, all.y=F, sort=FALSE)
c <- read.delim("~/cytoBand.txt", stringsAsFactors=FALSE, header=FALSE)
c$V6 = paste(c$V1, c$V4, sep="")
c$V6 = substr(c$V6, 4, nchar(c$V6))
imc <- merge(im, c, by.x="Cytoband", by.y="V6", all.x=T, all.y=F, sort=FALSE)
final = imc[ , c("V1", "V2", "V3", "Chromosome.scaffold.name", "Gene.start..bp.", "Gene.end..bp.")]
final$V1 = paste("hs", substr(final$V1, 4, nchar(final$V1)), sep="")
final$Chromosome.scaffold.name=paste("hs", as.character(final$Chromosome.scaffold.name), sep="")
write.table(final, "F1", sep="\t", quote=FALSE, row.names = FALSE, col.names=FALSE)

