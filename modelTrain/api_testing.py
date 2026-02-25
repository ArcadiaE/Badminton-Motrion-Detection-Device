import openai

openai.api_key = "YOUR API KEY"

try:
    response = openai.ChatCompletion.create(
        model="gpt-4o",
        messages=[{"role": "system", "content": "You're an assistant"},
                  {"role": "user", "content": "Test if OpenAI API is avaliable"}]
    )
    print(response["choices"][0]["message"]["content"])
except Exception as e:
    print(f"API request failed: {e}")

