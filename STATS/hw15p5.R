d1 = read.csv("SMMLevelsHW.csv", header=TRUE, stringsAsFactors=FALSE)
P = d1$clear_rate[which(d1$tag=="Puzzle")]
S = d1$clear_rate[which(d1$tag=="Speedrun")]
message("mean ", mean(P), " ", mean(S))
message("SD ", sd(P), " ", sd(S))
message("N ", length(P), " ", length(S))

