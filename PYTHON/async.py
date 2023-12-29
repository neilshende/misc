import asyncio

async def fetch_data(url):
    """Simulates fetching data from a URL."""
    print(f"Fetching data from {url}...")
    await asyncio.sleep(2)  # Simulate network delay
    print(f"Data fetched from {url}!")
    return f"Data fetched from {url}!\n"

async def main():
    tasks = [fetch_data("https://example.com/data1"), fetch_data("https://example.com/data2")]
    results = await asyncio.gather(*tasks)  # Gather results from multiple tasks
    print("All data fetched:", results)

if __name__ == "__main__":
    asyncio.run(main())
    print("done main")
