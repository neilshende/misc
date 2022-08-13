package main

import (
    "context"
    "go.mongodb.org/mongo-driver/mongo"
    "fmt"
)

func createRecord(collection *mongo.Collection, ctx context.Context, data map[string]interface{})(map[string]interface{}, error){

    req, err := collection.InsertOne(ctx, data)

    if err != nil {
        return nil, err
    }

    insertedId := req.InsertedID

    res := map[string]interface{}{
               "data" : map[string]interface{}{
                    "insertedId": insertedId,
                },
           }

    return res, nil
}

func createRecord2(collection *mongo.Collection)(error){

//try {
//collection.insertOne( {
//    product_id: 11,
//    product_name: "LEATHER BELT11",
//    retail_price: 24.3511
//  } )
//} catch (err) {
   fmt.Println("error inserting one")
//}
//return  err
return nil
}

