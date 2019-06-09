# babylist

- Difficulty: easy/medium
- Category: pwnable
- Vulnerability: While 'duplicating' the list, it memcpy's the structure which contains std::vector. This makes a two different lists pointing to the same elements storage. This can be converted to use after free and double frees.

## Testing

- docker build .
- sudo docker run -i -p 2301:2301 <image>
- python exploit.py