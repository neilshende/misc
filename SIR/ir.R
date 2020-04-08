library(ggplot2)
library(data.table)
library(readxl)
library(httr)
library(rvest)
library(XML)

url <- "https://www.worldometers.info/world-population/population-by-country/"
page = read_html(url, header=T, which=1,stringsAsFactors=F) 
tables <- html_nodes(page, "table") 
population <- html_table(tables)[[1]][, c(2, 3)]
names(population) <- c("Country", "Population")

url <- paste("https://www.ecdc.europa.eu/sites/default/files/documents/COVID-19-geographic-disbtribution-worldwide-",format(Sys.time(), "%Y-%m-%d"), ".xlsx", sep = "") 
GET(url, authenticate(":", ":", type="ntlm"), write_disk(tf <- "~/Downloads/delme.xlsx", overwrite = TRUE)) 
data <- read_excel(tf)

names(data)[7] <- "Country"
data$Country <- gsub("_", " ", data$Country) 
data <- setorder(data.table(data), Country, DateRep) 
data[, `:=`(cumCase = cumsum(Cases), cumDeath=cumsum(Deaths)), .(Country)] 
data[, maxCase := max(cumCase), .(Country)]

Start <- data[maxCase > 100,
             as.POSIXct(approx(log10(cumCase), DateRep, xout=log10(100))$y, origin="1970-01-01"),
             .(Country)]
data <- merge(data, Start)[, Days:= as.numeric(difftime(DateRep, V1, units= "days"))][Days >= 0]

C1 <- unique(data$Country)
C1 <- gsub("_", " ", C1)
C2 <- unique(population$Country)

matched_country <- NULL
for(i in C1) {
  for(j in C2)
     if(grepl(i, j) |grepl(j, i)) {
        matched_country <- rbind(matched_country, data.frame(C1=i, Country = j))
     }
}

population <- merge(population, matched_country)

population$Country <- NULL
temp <- names(population)
names(population)[which(temp == "C1")] <- "Country"
data <- merge(data, population)
data$Population <- as.numeric(gsub(",", "", data$Population)) 
data[, y := 1e6*cumCase/Population]

sel_countries <- c("China", "United States of America", "France",
                  "Italy", "Iran", "Japan", "South Korea", 'Germany',
                  "singapore", "India")

labels <- data[Country %in% sel_countries, .SD[.N], .(Country)]

ggplot(data[Country %in% sel_countries], aes(x = Days, y = y, color = Country)) +
    geom_line(show.legend= F) +
    geom_text(data=labels, aes(x= as.numeric(Days),
                               y= y, label = GeoId), nudge_y = 10e-2, show.legend= F) +
    scale_y_log10(labels = function(x) format(x, scientific = F)) +
    scale_x_continuous(breaks = seq(0, 100, by = 10)) +
    xlab("Days Since 100 Cases") +
    ylab("Incidence Rate (PPM)") +
    theme_bw() + annotation_logticks(sides = "l") +
    ggtitle(paste0("Cumulative Incidence Rate (Cases Per Million) of COVID19 as of ", Sys.Date()-1))

message("The End.")
ggplot(data[Country %in% sel_countries], aes(x = Days, y = y, color = Country)) +
    geom_line(show.legend= F) +
    geom_text(data=labels, aes(x= as.numeric(Days),
                               y= y, label = GeoId), nudge_y = 10e-2, show.legend= F) +
    scale_y_log10(labels = function(x) format(x, scientific = F)) +
    scale_x_continuous(breaks = seq(0, 100, by = 10)) +
    xlab("Days Since 100 Cases") +
    ylab("Incidence Rate (PPM)") +
    theme_bw() + annotation_logticks(sides = "l") +
    ggtitle(paste0("Cumulative Incidence Rate (Cases Per Million) of COVID19 as of ", Sys.Date()-1))
