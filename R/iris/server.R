#
# This is the server logic of a Shiny web application. You can run the 
# application by clicking 'Run App' above.
#
# Find out more about building applications with Shiny here:
# 
#    http://shiny.rstudio.com/
#

# Load libraries
library(shiny)
library(tree)
library(RColorBrewer)

# Create the server function
server <- function(input, output) {
  
  # Load the iris data set
  data(iris)
  
  # Set seed to make randomness reproducable
  set.seed(42)
  
  # Randomly sample 100 of 150 row indexes
  indexes <- sample(
    x = 1:150, 
    size = 100)
  
  # Create training set from indexes
  # NOTE: Need to use the superassignment operator here
  train <<- iris[indexes, ]
  
  # Create test set from remaining indexes
  test <- iris[-indexes, ]
  
  # Train tree model
  treeModel <- tree(
    formula = Species ~ .,
    data = train)
  
  # Create a color palette
  palette <- brewer.pal(3, "Set2")
  
  # Create render prediction function
  output$text = renderText({
    
    # Create predictors
    predictors <- data.frame(
      Petal.Length = input$petal.length,
      Petal.Width = input$petal.width,
      Sepal.Length = 0,
      Sepal.Width = 0)
    
    # Make prediction
    prediction = predict(
      object = treeModel,
      newdata = predictors,
      type = "class")
    
    # Create prediction text
    paste(
      "The predicted species is ",
      as.character(prediction))
  })
  
  # Create render plot function
  output$plot = renderPlot({
    
    # Create a scatterplot colored by species
    plot(
      x = iris$Petal.Length, 
      y = iris$Petal.Width,
      pch = 19,
      col = palette[as.numeric(iris$Species)],
      main = "Iris Petal Length vs. Width",
      xlab = "Petal Length (cm)",
      ylab = "Petal Width (cm)")
    
    # Plot the desicion boundaries
    partition.tree(
      treeModel,
      label = "Species",
      add = TRUE)
    
    # Draw predictor on plot
    points(
      x = input$petal.length,
      y = input$petal.width,
      col = "red",
      pch = 4,
      cex = 2,
      lwd = 2)
  })
}


ui <- fluidPage(
  titlePanel("Iris Species Predictor"),
  sidebarLayout(
    sidebarPanel(
      sliderInput(
        inputId = "petal.length",
        label = "Petal Length (cm)",
        min = 1,
        max = 7,
        value = 4),
      sliderInput(
        inputId = "petal.width",
        label = "Petal Width (cm)",
        min = 0.0,
        max = 2.5,
        step = 0.5,
        value = 1.5),
      p("To predict a species of iris flower, please select the petal length and the petal width using the two sliders above."),
      p("The predicted species will be displayed in the text on the top right. In addition, a data visualization in the bottom right will show how the decision was made. The lines represent the decision boundaries and the red X represents your selected input.")),
    mainPanel(
      textOutput(
        outputId = "text"),
      plotOutput(
        outputId = "plot"))))

shinyApp(ui = ui, server = server)
