package main
import (
    "net/http"
    "encoding/csv"
//    "encoding/json"
//    "fmt"
    "log"
    "os"
    "github.com/gin-gonic/gin"
)
type album struct {
    ID     string  `json:"id"`
    Title  string  `json:"title"`
    Artist string  `json:"artist"`
    Price  string `json:"price"`
}
//var albums = []album{
//    {ID: "1", Title: "Blue Train", Artist: "John Coltrane", Price: "56.99"},
//    {ID: "2", Title: "Jeru", Artist: "Gerry Mulligan", Price: "17.99"},
//    {ID: "3", Title: "Sarah Vaughan and Clifford Brown", Artist: "Sarah Vaughan", Price: "39.99"},
//}
var albums []album

func getAlbums(c *gin.Context) {
    c.IndentedJSON(http.StatusOK, albums)
}
func postAlbums(c *gin.Context) {
    var newAlbum album

    // Call BindJSON to bind the received JSON to
    // newAlbum.
    if err := c.BindJSON(&newAlbum); err != nil {
        return
    }

    // Add the new album to the slice.
    albums = append(albums, newAlbum)
    c.IndentedJSON(http.StatusCreated, newAlbum)
}
func addApi(c *gin.Context) {
   var newAlbum = album{c.Param("id"), c.Param("title"), c.Param("artist"), c.Param("price")}
   //newAlbum.ID := c.Param("id")
   //newAlbum.Title := c.Param("title")
   //newAlbum.Artist := c.Param("artist")
   //newAlbum.Price := c.Param("price")
   albums = append(albums, newAlbum)
   outputFile, err := os.OpenFile("data.csv", os.O_WRONLY|os.O_CREATE|os.O_APPEND, 0644)
   if err != nil {
       c.IndentedJSON(http.StatusNotFound, gin.H{"message": "unable to open csv"})
       return
   }
   defer outputFile.Close()
   writer := csv.NewWriter(outputFile)
   defer writer.Flush()
   var csvRow []string
   csvRow = append(csvRow, newAlbum.ID, newAlbum.Title, newAlbum.Artist, newAlbum.Price)
   if err := writer.Write(csvRow); err != nil {
         c.IndentedJSON(http.StatusNotFound, gin.H{"message": "failed to write a row"})
         return
   }
   c.IndentedJSON(http.StatusCreated, newAlbum)
}

func getAlbumByTitle(c *gin.Context) {
    title := c.Param("title")

    // Loop over the list of albums, looking for
    // an album whose ID value matches the parameter.
    for _, a := range albums {
        if a.Title == title {
            c.IndentedJSON(http.StatusOK, a)
            return
        }
    }
    c.IndentedJSON(http.StatusNotFound, gin.H{"message": "album not found"})
}
func getAlbumByID(c *gin.Context) {
    id := c.Param("id")

    // Loop over the list of albums, looking for
    // an album whose ID value matches the parameter.
    for _, a := range albums {
        if a.ID == id {
            c.IndentedJSON(http.StatusOK, a)
            return
        }
    }
    c.IndentedJSON(http.StatusNotFound, gin.H{"message": "album not found"})
}

func saveApi(c *gin.Context) {
    outputFile, err := os.Create("data.csv")
    if err != nil {
        c.IndentedJSON(http.StatusNotFound, gin.H{"message": "unable to create csv"})
        return
    }
    defer outputFile.Close()

    // 4. Write the header of the CSV file and the successive rows by iterating through the JSON struct array
    writer := csv.NewWriter(outputFile)
    defer writer.Flush()

    header := []string{"ID", "Title", "Artist", "Price"}
    if err := writer.Write(header); err != nil {
        c.IndentedJSON(http.StatusNotFound, gin.H{"message": "failed to write header"})
        return
    }

    for _, a := range albums {
       var csvRow []string
        csvRow = append(csvRow, a.ID, a.Title, a.Artist, a.Price)
        if err := writer.Write(csvRow); err != nil {
            c.IndentedJSON(http.StatusNotFound, gin.H{"message": "failed to write a row"})
            return
        }
    }
    c.IndentedJSON(http.StatusOK, albums)
}

// Usage

// TERM1
// go get .
// go run .

// TERM2
// curl http://localhost:8080/albums
// curl http://localhost:8080/albums     --include     --header "Content-Type: application/json"     --request "POST"     --data '{"id": "4","title": "The Modern Sound of Betty Carter","artist": "Betty Carter","price": "49.99"}'
// curl http://localhost:8080/albums
// curl http://localhost:8080/albums/id/2
// curl http://localhost:8080/albums/save
// curl http://localhost:8080/albums/add/id/5/title/foo/artist/me/price/100

func createList(data [][]string) { //[]album {
    // convert csv lines to array of structs
    //var albumList []album
    for i, line := range data {
        if i > 0 { // omit header line
            var rec album
            for j, field := range line {
                if j == 0 {
                    rec.ID = field
                } else if j == 1 {
                    rec.Title = field
                } else if j == 2 {
                    rec.Artist = field
                    //var err error
                    //rec.Rank, err = strconv.Atoi(field)
                    //if err != nil {
                    //    continue
                    //}
                } else if j == 3 {
                    rec.Price = field
                }
            }
            albums = append(albums, rec)
        }
    }
    //return albumList
}
func main() {
// open file
    f, err := os.Open("data.csv")
    if err != nil {
        log.Fatal(err)
    }

    // remember to close the file at the end of the program
    defer f.Close()

    // 2. Read CSV file using csv.Reader
    csvReader := csv.NewReader(f)
    data, err := csvReader.ReadAll()
    if err != nil {
        log.Fatal(err)
    }

    // 3. Assign successive lines of raw CSV data to fields of the created structs
    //albums := createList(data)
    createList(data)

    // 4. Convert an array of structs to JSON using marshaling functions from the encoding/json package
    //albums, err := json.MarshalIndent(list, "", "  ")
    //if err != nil {
    //    log.Fatal(err)
    //}

    //fmt.Println(string(albums))
    router := gin.Default()
    router.GET("/albums", getAlbums)
    router.GET("/albums/id/:id", getAlbumByID)
    router.GET("/albums/title/:title", getAlbumByTitle)
    router.GET("/albums/save", saveApi)
    router.GET("/albums/add/id/:id/title/:title/artist/:artist/price/:price", addApi)
    router.POST("/albums", postAlbums)
    router.Run("localhost:8080")
}

