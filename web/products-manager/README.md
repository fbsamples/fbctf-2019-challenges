## products-manager

- Description: This challenge is a database storage where users can create and view products (protected by secrets). There are already 5 products in the database, one of them has the flag in its description.

- Vulnerability: There is no length checking while creating products. SQL internally trims the column field depending on the maximum size. This can be used to create duplicate products. (See 'solve.py' for solution)

- Setup: docker-compose up

- Difficulty: easy

- Files to be distributed: dist/src.tar.gz
