#
# Reset environment
#

# if you script crash before finish you need force close serial conection and close file
# close(con)
# close(fileConn)

rm(list = ls())         # Remove environemnent variables
graphics.off()          # Close any open graphics

# Set here your directory to save your files 
setwd("C:/Users/felipe/OneDrive/DataProcessing/")

#
# Libraries
#

library(serial)
library(plotly)
#
# Script
#

con <- serialConnection(name = "test_con",
                        port = "COM13",
                        mode = "38400,n,8,1",
                        buffering = "none",
                        newline = 1,
                        translation = "cr")

open(con)

tempoEstimadoTeste <- (3600) * 1/60 #one minute 
stopTime <- Sys.time() + tempoEstimadoTeste

foo <- ""
textSize <- 0

# Abertura de arquivo para escrita de dados #
filename <- "output.txt"
fileConn <- file(filename)

file <- 1
i <- 1

aceleracaoX <- vector()
velocidadeX <- vector()
tempo <- vector()

aceleracaoX[i] <- 0
velocidadeX[i] <- 0
tempo[i] <- 0

teste <- 1

x11() #  To open new plot window
par(mfrow=c(2,1))

while(Sys.time() < stopTime)
{
  newText <- read.serialConnection(con)
  if(length(newText) > 0 && 0 < nchar(newText))
  {
    flush.console()
    foo <- paste(foo, newText)
    writeLines(foo,fileConn)
    
    ####   Desenho do grafico #####
    #browser()
    x.in <- textConnection(newText) #setup connection
    x.data <- read.table(x.in)      #read in the caracter vector
    
 
    for (k in seq(1,1,length(x.data$V1))) {
      aceleracaoX[i] <-  x.data$V1[k]
      velocidadeX[i] <- x.data$V2[k]
      tempo[i] <- i
      i <- i + 1
      }
    
    ## Plot first set of data and draw its axis
    
     plot(tempo, aceleracaoX, pch=4, axes=FALSE, ylim=c(-1,1), xlab="", ylab="", 
              type="p",col="red", main="Curva de Aceleracao no tempo")
    
      axis(2, ylim=c(-1,1),col="red",las=1)  ## las=1 makes horizontal labels
      axis(1)
      mtext("Aceleracao (mg)",side=2,line=2.5)
      mtext("Amostra N ",side=1,line=2.5)
      grid()
      box()
    
    
    
    ## Allow a second plot on otther graph
    
    
    plot(tempo, velocidadeX, axes=FALSE, ylim=c(-0.2,0.2), xlab="", ylab="", 
         type="p",col="blue", main="Curva de Velocidade no tempo")
    
    
      axis(2, ylim=c(-0.2,0.2),col="blue",las=1)  ## las=1 makes horizontal labels
      axis(1)
      mtext("Velocidade (m/s)",side=2,line=2.5)
      mtext("Amostra N",side=1,line=2.5)
      grid()
      box()
    
    
    
    
    ## Add Legend
    #legend("topright",legend=c("Aceleracao (mg)","Velocidade (m/s)"), rect(1,1),
    #text.col=c("red","blue"),pch=c(16,15),col=c("red","blue"))
    
    ## Draw the time axis
    #axis(1,pretty(range(tempo),10))
    #mtext("Time (sec)",side=1,col="black",line=2.5) 
  
    i<-i+1
  }
}

close(con)
close(fileConn)

