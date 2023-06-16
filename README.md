# Shared Likes

tables defined as:

- user with columns `user_id name` representing users

- interest with columns `interest_id title weight` representing possible interests for users

- like with columns `like_id user_id interest_id` representing a user's like of a particular interest

Objective: Given a particular user I want to get paged results of other users in descending order of most similarly liked interests. The solution needs to scale for 100k users and 100 interests.

The weight column indicates how much the interest should influence the result i.e an interest with weight of 3 should be 3x as influential as an interest with weight of 1.
